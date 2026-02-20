// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/WorkStationBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LKH2/CarryInteractComponent.h"
#include "LKH2/WorkstationData.h"

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

void AWorkStationBase::BeginPlay() { Super::BeginPlay(); }

void AWorkStationBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  if (WorkstationData && WorkstationData->WorkstationMesh) {
    RootMesh->SetStaticMesh(WorkstationData->WorkstationMesh);
  }
}

void AWorkStationBase::OnPickedUp_Implementation(AActor *Carrier) {
  // 워크스테이션은 고정형이므로 들려지지 않고 조작 인터랙션 수행
  if (InteractComponent) {
    InteractComponent->OnInteract(Carrier, WorkstationData);
  }
}

void AWorkStationBase::OnDropped_Implementation() {
  // 워크스테이션을 내려놓는 상황은 발생하지 않음
}

void AWorkStationBase::OnThrown_Implementation(FVector ThrowVelocity) {
  // 워크스테이션을 던지는 상황은 발생하지 않음
}

void AWorkStationBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  // 워크스테이션 외곽선 강조 처리
  if (RootMesh) {
    RootMesh->SetRenderCustomDepth(bEnabled);
    RootMesh->SetCustomDepthStencilValue(1);
  }
}
