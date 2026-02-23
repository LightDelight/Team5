// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/WorkStation/WorkStationBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "LKH2/WorkStation/WorkstationData.h"
#include "Net/UnrealNetwork.h"

AWorkStationBase::AWorkStationBase() {
  PrimaryActorTick.bCanEverTick = false;

  // 본 액터 리플리케이션 활성화 (고정형이므로 Movement는 기본 false)
  bReplicates = true;

  // 고정형 스태틱 메쉬 중심 설계
  RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
  RootComponent = RootMesh;

  // 메쉬 자체의 물리/충돌 비활성화 (오직 시각 요소)
  RootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // 박스 충돌체
  BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
  BoxCollision->SetupAttachment(RootMesh);

  // 충돌 설정 (플레이어 및 다른 물리 객체를 막거나 인터랙션 감지용)
  BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  BoxCollision->SetCollisionResponseToAllChannels(ECR_Block);

  // 워크스테이션도 아웃라인 및 감지에 반응해야 하므로 관련된 Overlap 채널 설정
  // CarryComponent의 DetectionSphere가 Overlap 할 수 있는 채널로 설정 (예:
  // ECC_PhysicsBody 이나 필요시 Custom Channel 할당)
  BoxCollision->SetCollisionObjectType(ECC_PhysicsBody);

  InteractComponent = CreateDefaultSubobject<UCarryInteractComponent>(
      TEXT("InteractComponent"));
  InteractComponent->SetupAttachment(RootMesh);
}

void AWorkStationBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AWorkStationBase, WorkstationData);
}

void AWorkStationBase::OnRep_WorkstationData() {
  // 클라이언트에서 WorkstationData가 리플리케이트되면 메쉬 적용
  if (WorkstationData && WorkstationData->WorkstationMesh) {
    RootMesh->SetStaticMesh(WorkstationData->WorkstationMesh);
  }

  // 박스 콜리전 크기 및 상대 좌표 적용
  if (WorkstationData && BoxCollision) {
    BoxCollision->SetBoxExtent(WorkstationData->BoxExtent);
    BoxCollision->SetRelativeLocation(WorkstationData->BoxRelativeLocation);
  }

  // 로직 모듈 초기화 (Display 액터 등)
  if (WorkstationData) {
    for (ULogicModuleBase *Module : WorkstationData->GetAllModules()) {
      if (Module) {
        Module->InitializeLogic(this);
      }
    }
  }
}

void AWorkStationBase::BeginPlay() {
  Super::BeginPlay();

  // 모든 로직 모듈에 런타임 초기화 기회 제공
  if (WorkstationData) {
    for (ULogicModuleBase *Module : WorkstationData->GetAllModules()) {
      if (Module) {
        Module->InitializeLogic(this);
      }
    }
  }
}

void AWorkStationBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // 그리드 스냅: 에디터에서 배치/이동 시 셀 중심에 자동 정렬
  if (bSnapToGrid && SnapCellSize > 0.0f) {
    FVector Loc = GetActorLocation();
    FVector Relative = Loc - SnapGridOrigin;

    // GridManager와 동일한 좌표 변환: Floor → 셀 중심 계산
    int32 GridX = FMath::FloorToInt32(Relative.X / SnapCellSize);
    int32 GridY = FMath::FloorToInt32(Relative.Y / SnapCellSize);

    float SnappedX = SnapGridOrigin.X + (GridX + 0.5f) * SnapCellSize;
    float SnappedY = SnapGridOrigin.Y + (GridY + 0.5f) * SnapCellSize;

    SetActorLocation(FVector(SnappedX, SnappedY, Loc.Z));
  }

  if (WorkstationData && WorkstationData->WorkstationMesh) {
    RootMesh->SetStaticMesh(WorkstationData->WorkstationMesh);
  }

  // 박스 콜리전 크기 및 상대 좌표 적용
  if (WorkstationData && BoxCollision) {
    BoxCollision->SetBoxExtent(WorkstationData->BoxExtent);
    BoxCollision->SetRelativeLocation(WorkstationData->BoxRelativeLocation);
  }

  // 모든 로직 모듈에 에디터 미리보기 기회 제공
  if (WorkstationData) {
    for (ULogicModuleBase *Module : WorkstationData->GetAllModules()) {
      if (Module) {
        Module->OnConstructionLogic(this);
      }
    }
  }
}

bool AWorkStationBase::OnCarryInteract_Implementation(
    AActor *Interactor, ECarryInteractionType InteractionType) {
  if (InteractComponent && WorkstationData) {
    return InteractComponent->OnInteract(Interactor, WorkstationData,
                                         InteractionType);
  }
  return false;
}

UCarryableComponent *AWorkStationBase::GetCarryableComponent() const {
  return nullptr; // 워크스테이션 자체는 들려지지 않으므로 nullptr 반환
}

UCarryInteractComponent *AWorkStationBase::GetCarryInteractComponent() const {
  return InteractComponent;
}

FLogicBlackboard *AWorkStationBase::GetLogicBlackboard() {
  if (InteractComponent) {
    return &InteractComponent->LogicBlackboard;
  }
  return nullptr;
}

void AWorkStationBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  // 워크스테이션 외곽선 강조 처리
  if (RootMesh) {
    RootMesh->SetRenderCustomDepth(bEnabled);
    RootMesh->SetCustomDepthStencilValue(1);
  }
}
