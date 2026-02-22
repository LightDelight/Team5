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

  if (!Cell->IsEmpty())
    return false;

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

  // 4. 워크스테이션 스폰 및 등록
  // SpawnActorDeferred를 사용하여 WorkstationData를 BeginPlay 전에 설정.
  // (SpawnActor 사용 시 BeginPlay가 먼저 실행되어 LogicModule 초기화 누락)
  for (const FMapWorkstationEntry &Entry : MapData->Workstations) {
    if (!Entry.StationClass || !IsValidCoord(Entry.GridCoord))
      continue;

    FVector SpawnLocation = GridToWorld(Entry.GridCoord);
    FTransform SpawnTransform(Entry.Rotation, SpawnLocation);

    AWorkStationBase *NewStation = World->SpawnActorDeferred<AWorkStationBase>(
        Entry.StationClass, SpawnTransform);
    if (!NewStation)
      continue;

    // WorkstationData 설정 (FinishSpawning/BeginPlay 전에 설정해야 LogicModule 초기화 정상 동작)
    if (Entry.StationData) {
      if (FProperty *Prop = NewStation->GetClass()->FindPropertyByName(
              TEXT("WorkstationData"))) {
        if (FObjectPropertyBase *ObjProp =
                CastField<FObjectPropertyBase>(Prop)) {
          ObjProp->SetObjectPropertyValue_InContainer(NewStation,
                                                      Entry.StationData);
        }
      }
    }

    NewStation->FinishSpawning(SpawnTransform);

    // 그리드에 등록
    RegisterActor(NewStation, Entry.GridCoord);
    SpawnedActors.Add(NewStation);
  }

  UE_LOG(LogTemp, Log,
         TEXT("GridManager: MapData로부터 %d개 워크스테이션을 로드했습니다."),
         SpawnedActors.Num());
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

