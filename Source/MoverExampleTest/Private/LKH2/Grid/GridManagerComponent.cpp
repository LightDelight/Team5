// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Grid/GridManagerComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "LKH2/Grid/MapData.h"
#include "LKH2/WorkStation/WorkStationBase.h"
#include "LKH2/WorkStation/WorkstationData.h"

#if WITH_EDITOR
#include "UObject/SavePackage.h"
#endif

UGridManagerComponent::UGridManagerComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  SetIsReplicatedByDefault(true);
}

void UGridManagerComponent::BeginPlay() {
  Super::BeginPlay();

  if (bBakeOnBeginPlay && MapData) {
    // 레벨에 이미 배치된 워크스테이션을 스캔하여 MapData에 저장
    InitializeGrid();
    BakeCurrentLevel();
    bBakeOnBeginPlay = false;
    UE_LOG(LogTemp, Log,
           TEXT("GridManager: bBakeOnBeginPlay 완료. "
                "다음 실행부터는 MapData에서 로드됩니다."));
  } else if (MapData) {
    // 베이크된 MapData로부터 워크스테이션 스폰
    LoadFromMapData();
  } else {
    InitializeGrid();
  }
}

void UGridManagerComponent::InitializeGrid() {
  Cells.Empty();
  Cells.Reserve(GridWidth * GridHeight);

  for (int32 Y = 0; Y < GridHeight; ++Y) {
    for (int32 X = 0; X < GridWidth; ++X) {
      FIntPoint Coord(X, Y);
      FVector Center = GridToWorld(Coord);
      Cells.Add(Coord, FGridCell(Coord, Center));
    }
  }
}

// ── 좌표 변환 ──

FIntPoint UGridManagerComponent::WorldToGrid(
    const FVector &WorldLocation) const {
  FVector Relative = WorldLocation - GridOrigin;
  int32 X = FMath::FloorToInt32(Relative.X / CellSize);
  int32 Y = FMath::FloorToInt32(Relative.Y / CellSize);
  return FIntPoint(X, Y);
}

FVector UGridManagerComponent::GridToWorld(const FIntPoint &GridCoord) const {
  float WorldX = GridOrigin.X + (GridCoord.X + 0.5f) * CellSize;
  float WorldY = GridOrigin.Y + (GridCoord.Y + 0.5f) * CellSize;
  return FVector(WorldX, WorldY, GridOrigin.Z);
}

// ── 유효성 검사 ──

bool UGridManagerComponent::IsValidCoord(const FIntPoint &GridCoord) const {
  return GridCoord.X >= 0 && GridCoord.X < GridWidth && GridCoord.Y >= 0 &&
         GridCoord.Y < GridHeight;
}

bool UGridManagerComponent::IsCellEmpty(const FIntPoint &GridCoord) const {
  const FGridCell *Cell = Cells.Find(GridCoord);
  return Cell ? Cell->IsEmpty() : true;
}

FGridCell UGridManagerComponent::GetCell(const FIntPoint &GridCoord) const {
  const FGridCell *Cell = Cells.Find(GridCoord);
  return Cell ? *Cell : FGridCell();
}

// ── 등록 / 해제 ──

bool UGridManagerComponent::RegisterActor(AActor *Actor,
                                          const FIntPoint &GridCoord) {
  if (!Actor || !IsValidCoord(GridCoord))
    return false;

  FGridCell *Cell = Cells.Find(GridCoord);
  if (!Cell)
    return false;

  if (!Cell->IsEmpty()) {
    if (Cell->OccupyingActor == Actor) {
      return true; // 이미 본인이 등록되어 있으면 성공으로 간주
    }
    return false;
  }

  Cell->OccupyingActor = Actor;
  return true;
}

bool UGridManagerComponent::RegisterActorAtWorldLocation(
    AActor *Actor, const FVector &WorldLocation) {
  FIntPoint Coord = WorldToGrid(WorldLocation);
  return RegisterActor(Actor, Coord);
}

void UGridManagerComponent::UnregisterActor(const FIntPoint &GridCoord) {
  FGridCell *Cell = Cells.Find(GridCoord);
  if (Cell) {
    Cell->OccupyingActor = nullptr;
  }
}

// ── 쿼리 ──

AActor *UGridManagerComponent::GetActorAt(const FIntPoint &GridCoord) const {
  const FGridCell *Cell = Cells.Find(GridCoord);
  if (Cell && Cell->OccupyingActor.IsValid()) {
    return Cell->OccupyingActor.Get();
  }
  return nullptr;
}

AActor *UGridManagerComponent::GetActorAtWorldLocation(
    const FVector &WorldLocation) const {
  FIntPoint Coord = WorldToGrid(WorldLocation);
  return GetActorAt(Coord);
}

TArray<AActor *> UGridManagerComponent::GetNearbyActors(
    const FVector &WorldLocation, int32 Radius) const {
  TArray<AActor *> Result;
  FIntPoint CenterCoord = WorldToGrid(WorldLocation);

  for (int32 DY = -Radius; DY <= Radius; ++DY) {
    for (int32 DX = -Radius; DX <= Radius; ++DX) {
      FIntPoint CheckCoord(CenterCoord.X + DX, CenterCoord.Y + DY);
      if (!IsValidCoord(CheckCoord))
        continue;

      AActor *Actor = GetActorAt(CheckCoord);
      if (Actor) {
        Result.Add(Actor);
      }
    }
  }

  return Result;
}

// ── 맵 베이킹 파이프라인 ──

void UGridManagerComponent::BakeCurrentLevel() {
#if WITH_EDITOR
  if (!MapData) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager: MapData 에셋이 설정되지 않았습니다. "
                "먼저 MapData를 생성하고 할당해주세요."));
    return;
  }

  UWorld *World = GetWorld();
  if (!World) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager: 유효한 월드를 찾을 수 없습니다."));
    return;
  }  // 1. 레벨의 모든 워크스테이션 스캔
  TArray<AActor *> FoundStations;
  UGameplayStatics::GetAllActorsOfClass(World, AWorkStationBase::StaticClass(),
                                        FoundStations);

  // 2. 현재 그리드 설정을 MapData에 복사
  MapData->GridOrigin = GridOrigin;
  MapData->CellSize = CellSize;
  MapData->GridWidth = GridWidth;
  MapData->GridHeight = GridHeight;

  // 3. 워크스테이션 목록 생성
  MapData->Workstations.Empty();
  int32 BakedCount = 0;

  for (AActor *Actor : FoundStations) {
    AWorkStationBase *Station = Cast<AWorkStationBase>(Actor);
    if (!Station)
      continue;

    FMapWorkstationEntry Entry;
    Entry.GridCoord = WorldToGrid(Station->GetActorLocation());
    Entry.StationClass = Station->GetClass();
    Entry.Rotation = Station->GetActorRotation();

    // WorkstationData 접근 (Reflection으로 안전하게)
    if (FProperty *Prop =
            Station->GetClass()->FindPropertyByName(TEXT("WorkstationData"))) {
      if (FObjectPropertyBase *ObjProp = CastField<FObjectPropertyBase>(Prop)) {
        Entry.StationData =
            Cast<UWorkstationData>(ObjProp->GetObjectPropertyValue_InContainer(Station));
      }
    }

    if (!IsValidCoord(Entry.GridCoord)) {
      UE_LOG(LogTemp, Warning,
             TEXT("GridManager: %s 의 좌표 (%d, %d) 가 그리드 범위 밖입니다. "
                  "건너뜁니다."),
             *Station->GetName(), Entry.GridCoord.X, Entry.GridCoord.Y);
      continue;
    }

    MapData->Workstations.Add(Entry);
    ++BakedCount;
  }

  // 4. 에셋 저장
  MapData->MarkPackageDirty();

  UE_LOG(LogTemp, Log,
         TEXT("GridManager: 베이크 완료! %d개 워크스테이션을 MapData에 "
              "저장했습니다. (그리드: %dx%d, 셀: %.0fcm)"),
         BakedCount, GridWidth, GridHeight, CellSize);

#else
  UE_LOG(LogTemp, Warning,
         TEXT("GridManager: BakeCurrentLevel은 에디터에서만 사용 가능합니다."));
#endif
}

void UGridManagerComponent::LoadFromMapData() {
  if (!MapData) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager: MapData가 설정되지 않았습니다."));
    return;
  }

  UWorld *World = GetWorld();
  if (!World)
    return;

  // 1. MapData의 그리드 설정 적용
  GridOrigin = MapData->GridOrigin;
  CellSize = MapData->CellSize;
  GridWidth = MapData->GridWidth;
  GridHeight = MapData->GridHeight;

  // 2. 그리드 초기화
  InitializeGrid();

  // 3. 기존 스폰 액터 정리
  for (AActor *Actor : SpawnedActors) {
    if (IsValid(Actor)) {
      Actor->Destroy();
    }
  }
  SpawnedActors.Empty();

  // 4. 워크스테이션 스폰 및 등록 (SpawnWorkstation API 활용)
  for (const FMapWorkstationEntry &Entry : MapData->Workstations) {
    SpawnWorkstation(Entry.StationClass, Entry.StationData, Entry.GridCoord,
                     Entry.Rotation);
  }

  UE_LOG(LogTemp, Log,
         TEXT("GridManager: MapData로부터 %d개 워크스테이션을 로드했습니다."),
         SpawnedActors.Num());
}

// ── 런타임 워크스테이션 스폰 ──

AWorkStationBase *UGridManagerComponent::SpawnWorkstation(
    TSubclassOf<AWorkStationBase> StationClass,
    UWorkstationData *StationData, const FIntPoint &GridCoord,
    const FRotator &Rotation) {
  if (!StationClass) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager::SpawnWorkstation - StationClass가 nullptr입니다."));
    return nullptr;
  }

  if (!IsValidCoord(GridCoord)) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager::SpawnWorkstation - 좌표 (%d, %d)가 그리드 범위 "
                "밖입니다."),
           GridCoord.X, GridCoord.Y);
    return nullptr;
  }

  if (!IsCellEmpty(GridCoord)) {
    UE_LOG(LogTemp, Warning,
           TEXT("GridManager::SpawnWorkstation - 좌표 (%d, %d)가 이미 "
                "점유되어 있습니다."),
           GridCoord.X, GridCoord.Y);
    return nullptr;
  }

  UWorld *World = GetWorld();
  if (!World)
    return nullptr;

  FVector SpawnLocation = GridToWorld(GridCoord);
  FTransform SpawnTransform(Rotation, SpawnLocation);

  AWorkStationBase *NewStation = nullptr;

  if (GetOwner()->HasAuthority()) {
    // [Server] 실제로 스택틱 액터 스폰
    NewStation = World->SpawnActorDeferred<AWorkStationBase>(StationClass,
                                                             SpawnTransform);
    if (NewStation) {
      NewStation->SetWorkstationDataAndApply(StationData);
      NewStation->FinishSpawning(SpawnTransform);
    }
  } else {
    // [Client] 리플리케이트된 액터를 찾아서 그리드에 등록만 시도
    TArray<AActor *> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, StationClass, FoundActors);

    for (AActor *Actor : FoundActors) {
      if (Actor->GetActorLocation().Equals(SpawnLocation, 10.0f)) {
        NewStation = Cast<AWorkStationBase>(Actor);
        break;
      }
    }

    if (NewStation) {
      UE_LOG(LogTemp, Log,
             TEXT("GridManager: 클라이언트에서 위치 (%d, %d)의 리플리케이트된 "
                  "액터를 찾았습니다."),
             GridCoord.X, GridCoord.Y);
    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("GridManager: 클라이언트에서 위치 (%d, %d)의 리플리케이트된 "
                  "액터를 찾지 못했습니다. (지연될 수 있음)"),
             GridCoord.X, GridCoord.Y);
    }
  }

  if (NewStation) {
    // 그리드에 등록 및 추적
    // (AWorkStationBase::BeginPlay에서도 등록을 시도하지만, 여기서 중복 체크를
    // 수행하므로 안전함)
    RegisterActor(NewStation, GridCoord);
    SpawnedActors.AddUnique(NewStation);
  }

  return NewStation;
}

bool UGridManagerComponent::DestroyWorkstation(const FIntPoint &GridCoord) {
  if (!IsValidCoord(GridCoord))
    return false;

  AActor *Actor = GetActorAt(GridCoord);
  if (!Actor)
    return false;

  // 추적 목록에서 제거
  SpawnedActors.Remove(Actor);

  // 그리드에서 해제
  UnregisterActor(GridCoord);

  // 액터 파괴
  Actor->Destroy();

  return true;
}

void UGridManagerComponent::ClearGrid() {
  // 스폰된 액터 제거
  for (AActor *Actor : SpawnedActors) {
    if (IsValid(Actor)) {
      Actor->Destroy();
    }
  }
  SpawnedActors.Empty();

  // 그리드 재초기화 (셀 비움)
  InitializeGrid();

  UE_LOG(LogTemp, Log,
         TEXT("GridManager: 그리드를 초기화했습니다. (%dx%d)"),
         GridWidth, GridHeight);
}

