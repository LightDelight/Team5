// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Item/ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryableComponent.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Item/ItemSmoothingComponent.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "Net/UnrealNetwork.h"

void AItemBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME_CONDITION_NOTIFY(AItemBase, ItemData, COND_None,
                                 REPNOTIFY_Always);
}

void AItemBase::OnRep_ItemData() { SetItemDataAndApply(ItemData); }

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

bool AItemBase::OnCarryInteract_Implementation(
    AActor *Interactor, ECarryInteractionType InteractionType) {
  if (!StateComponent || !CarryComponent) {
    return false;
  }

  // 액터 본연의 책임을 벗어나던 물리 및 상태 강제 전이(Prediction/Rollback)
  // 로직을 모두 제거. 실제 줍기/내려놓기/투척(상태 전이, 부착, 물리 설정)은
  // 모두 ICarryLogicInterface(모듈)에게 위임합니다.
  return CarryComponent->OnCarryInteract(Interactor, InteractionType);
}

void AItemBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  if (CarryComponent) {
    CarryComponent->SetOutlineEnabled(bEnabled);
  }
}

void AItemBase::SetItemDataAndApply(UItemData *InData) {
  ItemData = InData;
  if (ItemData) {
    if (ItemData->ItemMesh && VisualMesh) {
      VisualMesh->SetStaticMesh(ItemData->ItemMesh);
    }
    if (SphereCollision) {
      SphereCollision->SetMassOverrideInKg(NAME_None, ItemData->ItemWeight,
                                           true);
      SphereCollision->SetSphereRadius(ItemData->SphereRadius);
    }
    if (VisualMesh) {
      VisualMesh->SetRelativeLocation(ItemData->MeshRelativeLocation);
    }
  }
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  if (ItemData) {
    // 메쉬 적용 (VisualMesh)
    if (ItemData->ItemMesh && VisualMesh) {
      VisualMesh->SetStaticMesh(ItemData->ItemMesh);
    }

    // 무게(질량) 및 콜리전 크기 적용
    if (SphereCollision) {
      SphereCollision->SetMassOverrideInKg(NAME_None, ItemData->ItemWeight,
                                           true);
      SphereCollision->SetSphereRadius(ItemData->SphereRadius);
    }

    // 메쉬 상대 좌표 적용
    if (VisualMesh) {
      VisualMesh->SetRelativeLocation(ItemData->MeshRelativeLocation);
    }
  }
}