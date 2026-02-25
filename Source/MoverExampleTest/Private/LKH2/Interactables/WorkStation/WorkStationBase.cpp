#include "LKH2/Interactables/WorkStation/WorkStationBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Grid/GridManagerComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interactables/WorkStation/WorkstationData.h"
#include "Net/UnrealNetwork.h"

AWorkStationBase::AWorkStationBase() {
  PrimaryActorTick.bCanEverTick = false;
  bReplicates = true;

  RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
  RootComponent = RootMesh;
  RootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
  BoxCollision->SetupAttachment(RootMesh);
  BoxCollision->SetCollisionProfileName(TEXT("PhysicsActor"));

  InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(
      TEXT("InteractableComponent"));
  InteractableComponent->SetupAttachment(RootMesh);

  BlackboardComponent = CreateDefaultSubobject<ULogicContextComponent>(
      TEXT("BlackboardComponent"));
}

void AWorkStationBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AWorkStationBase, WorkstationData);
}

void AWorkStationBase::BeginPlay()
{
  Super::BeginPlay();

  // 1. 모든 로직 모듈 초기화 (LogicContextComponent에 위임)
  if (BlackboardComponent) 
    {
    BlackboardComponent->InitializeLogic(WorkstationData, this);
    }

  // 2. 클라이언트 사이드 그리드 자동 등록 보완
  // GridManager가 클라이언트에서 스폰 액터를 제때 찾지 못했을 경우를 대비
  UWorld* World = GetWorld();
  if (World)
  {
    if (!HasAuthority())
    {
      if (AGameStateBase *GameState = GetWorld()->GetGameState())
      {
        if (UGridManagerComponent *GridManager =
                GameState->FindComponentByClass<UGridManagerComponent>())              
        {
          GridManager->RegisterActorAtWorldLocation(this, GetActorLocation());
        }
      }
    }
  }
}

void AWorkStationBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // 1. 그리드 스냅 (에디터 전용)
  if (bSnapToGrid && !GetWorld()->IsGameWorld()) {
    FVector CurrentLoc = GetActorLocation();
    FVector RelativeLoc = CurrentLoc - SnapGridOrigin;

    float SnappedX = FMath::GridSnap(RelativeLoc.X, SnapCellSize);
    float SnappedY = FMath::GridSnap(RelativeLoc.Y, SnapCellSize);

    FVector SnappedLoc =
        FVector(SnappedX, SnappedY, RelativeLoc.Z) + SnapGridOrigin;
    SetActorLocation(SnappedLoc);
  }

  // 2. 통합 데이터 적용 (메쉬, 콜리전, 오프셋)
  SetWorkstationDataAndApply(WorkstationData);

  // 3. 모든 로직 모듈에 에디터 미리보기 기회 제공
  if (BlackboardComponent) {
    BlackboardComponent->InitializeLogic(WorkstationData, this);
  }

  for (ULogicModuleBase *Module : GetLogicModules()) {
    if (Module) {
      Module->OnConstructionLogic(this);
    }
  }
}

void AWorkStationBase::SetWorkstationDataAndApply(UWorkstationData *InData) {
  WorkstationData = InData;
  if (WorkstationData) {
    // 메쉬 적용
    UStaticMesh *Mesh = WorkstationData->GetEffectiveWorkstationMesh();
    if (Mesh && RootMesh) {
      RootMesh->SetStaticMesh(Mesh);
    }

    // 박스 콜리전 크기 및 상대 좌표 적용
    if (BoxCollision) {
      BoxCollision->SetBoxExtent(WorkstationData->GetEffectiveBoxExtent());
      BoxCollision->SetRelativeLocation(
          WorkstationData->GetEffectiveBoxRelativeLocation());
    }

    // InteractableComponent 오프셋 적용
    if (InteractableComponent) {
      InteractableComponent->SetRelativeLocation(
          WorkstationData->GetEffectiveInteractRelativeLocation());
    }
  }
}

void AWorkStationBase::OnRep_WorkstationData() {
  // 클라이언트 복제 시 데이터 적용
  SetWorkstationDataAndApply(WorkstationData);

  // 로직 모듈 런타임 초기화 (클라이언트 사이드)
  if (BlackboardComponent) {
    BlackboardComponent->InitializeLogic(WorkstationData, this);
  }
}

UInteractableComponent *AWorkStationBase::GetInteractableComponent() const {
  return InteractableComponent;
}

FLogicBlackboard *AWorkStationBase::GetLogicBlackboard() {
  return BlackboardComponent ? BlackboardComponent->GetBlackboard() : nullptr;
}

const FItemStatValue *AWorkStationBase::FindStat(const FGameplayTag &Tag) const {
  return BlackboardComponent ? BlackboardComponent->FindStat(Tag) : nullptr;
}

void AWorkStationBase::SetStat(const FGameplayTag &Tag,
                               const FItemStatValue &Value) {
  if (BlackboardComponent) {
    BlackboardComponent->SetStat(Tag, Value);
  }
}

FGameplayTag AWorkStationBase::ResolveKey(const FGameplayTag &Key) const {
  return BlackboardComponent ? BlackboardComponent->ResolveKey(Key) : Key;
}

TArray<ULogicModuleBase *> AWorkStationBase::GetLogicModules() const {
  return BlackboardComponent ? BlackboardComponent->GetLogicModules()
                             : TArray<ULogicModuleBase *>();
}

bool AWorkStationBase::OnInteract_Implementation(const FInteractionContext &Context) {
  if (InteractableComponent) {
    return InteractableComponent->OnInteract(Context);
  }
  return false;
}

void AWorkStationBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  if (RootMesh) {
    RootMesh->SetRenderCustomDepth(bEnabled);
    RootMesh->SetCustomDepthStencilValue(1);
  }
}
