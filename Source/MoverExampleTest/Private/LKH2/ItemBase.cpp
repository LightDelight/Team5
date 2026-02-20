// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "LKH2/CarryableComponent.h"
#include "LKH2/CarryableInterface.h"
#include "LKH2/ItemData.h"
#include "LKH2/ItemSmoothingComponent.h"
#include "LKH2/ItemStateComponent.h"
#include "Net/UnrealNetwork.h"

// 기본값 설정
AItemBase::AItemBase() {
  // 이 액터가 프레임마다 Tick()을 호출하도록 설정합니다. 성능 향상을 위해 필요
  // 없다면 끌 수 있습니다.
  PrimaryActorTick.bCanEverTick = true;

  // 리플리케이션 설정
  bReplicates = true;
  SetReplicateMovement(true);

  // 물리 충돌체 생성 (Sphere Setup)
  SphereCollision =
      CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
  RootComponent = SphereCollision;

  // 물리 설정
  SphereCollision->InitSphereRadius(32.0f); // 기본 반지름 설정
  SphereCollision->SetSimulatePhysics(true);
  SphereCollision->SetCollisionProfileName(
      TEXT("PhysicsActor")); // 표준 물리 프로필
  SphereCollision->SetGenerateOverlapEvents(true);

  // 세팅: 밀리긴 하지만 튕기거나 미끄러지지는 않도록 (바운스 없음, 높은
  // 마찰/감쇠)
  SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  SphereCollision->SetCollisionResponseToAllChannels(ECR_Block);

  // 회전 잠금 Z축만 허용
  SphereCollision->BodyInstance.bLockXRotation = true;
  SphereCollision->BodyInstance.bLockYRotation = true;
  SphereCollision->BodyInstance.bLockZRotation =
      false; // 기본 보행처럼 회전 가능

  // 댐핑 설정으로 미끄럽지 않게
  SphereCollision->SetLinearDamping(1.0f);
  SphereCollision->SetAngularDamping(2.0f);

  // 시각적 메쉬 생성
  VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
  VisualMesh->SetupAttachment(SphereCollision);
  VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 시각 전용

  StateComponent =
      CreateDefaultSubobject<UItemStateComponent>(TEXT("StateComponent"));
  SmoothingComponent = CreateDefaultSubobject<UItemSmoothingComponent>(
      TEXT("SmoothingComponent"));
  CarryComponent =
      CreateDefaultSubobject<UCarryableComponent>(TEXT("CarryComponent"));
}

// 게임이 시작되거나 스폰될 때 호출됩니다
void AItemBase::BeginPlay() {
  Super::BeginPlay();

  if (SmoothingComponent) {
    SmoothingComponent->InitialSetup(SphereCollision, VisualMesh);
  }
}

void AItemBase::OnPickedUp_Implementation(AActor *Carrier) {
  if (StateComponent) {
    StateComponent->SetItemState(EItemState::Carried);
  }
  if (CarryComponent) {
    CarryComponent->OnPickedUp(Carrier);
  }
}

void AItemBase::OnDropped_Implementation() {
  if (StateComponent) {
    StateComponent->SetItemState(EItemState::Placed);
  }
  if (CarryComponent) {
    CarryComponent->OnDropped();
  }
}

void AItemBase::OnThrown_Implementation(FVector ThrowVelocity) {
  if (StateComponent) {
    StateComponent->SetItemState(
        EItemState::Placed); // 던져진 후엔 기본적으로 맵에 배치된 상태로 취급
                             // (필요 시 수정)
  }
  if (CarryComponent) {
    CarryComponent->OnThrown(ThrowVelocity);
  }
}

void AItemBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  if (CarryComponent) {
    CarryComponent->SetOutlineEnabled(bEnabled);
  }
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  if (ItemData) {
    // 메쉬 적용 (VisualMesh)
    if (ItemData->ItemMesh) {
      VisualMesh->SetStaticMesh(ItemData->ItemMesh);
      // 참고: SphereCollision의 크기는 여기서 메쉬에 맞춰 자동으로 조정되지
      // 않으므로, 필요하다면 메쉬 bounds를 기반으로 SphereRadius를 조정하는
      // 로직을 추가할 수 있습니다.
    }

    // 무게(질량) 적용
    if (SphereCollision) {
      SphereCollision->SetMassOverrideInKg(NAME_None, ItemData->ItemWeight,
                                           true);
    }
  }
}