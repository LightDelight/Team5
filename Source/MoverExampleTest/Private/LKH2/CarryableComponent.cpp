// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/CarryableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/CarryableLogicInterface.h"
#include "LKH2/ItemBase.h"
#include "LKH2/ItemData.h"
#include "LKH2/LogicModuleBase.h"

UCarryableComponent::UCarryableComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  bIsCarried = false;
  CurrentCarrier = nullptr;
}

void UCarryableComponent::BeginPlay() { Super::BeginPlay(); }

void UCarryableComponent::OnPickedUp(AActor *Carrier) {
  // 경쟁 상태(Race Condition) 방어: 이미 누군가 들고 있다면 무시
  if (bIsCarried || CurrentCarrier != nullptr) {
    return;
  }

  bIsCarried = true;
  CurrentCarrier = Carrier;

  // ItemData가 가진 추가 로직(무게에 따른 이동속도 저하 등) 호출
  if (AItemBase *ItemOwner = Cast<AItemBase>(GetOwner())) {
    if (UItemData *Data = ItemOwner->GetItemData()) {
      for (ULogicModuleBase *Module : Data->LogicModules) {
        if (Module && Module->Implements<UCarryableLogicInterface>()) {
          ICarryableLogicInterface::Execute_OnModulePickedUp(Module, Carrier,
                                                             ItemOwner);
        }
      }
    }
  }
}

void UCarryableComponent::OnDropped() {
  if (!bIsCarried)
    return;

  AActor *LastCarrier = CurrentCarrier;
  bIsCarried = false;
  CurrentCarrier = nullptr;

  if (AItemBase *ItemOwner = Cast<AItemBase>(GetOwner())) {
    if (UItemData *Data = ItemOwner->GetItemData()) {
      for (ULogicModuleBase *Module : Data->LogicModules) {
        if (Module && Module->Implements<UCarryableLogicInterface>()) {
          ICarryableLogicInterface::Execute_OnModuleDropped(Module, LastCarrier,
                                                            ItemOwner);
        }
      }
    }
  }
}

void UCarryableComponent::OnThrown(FVector ThrowVelocity) {
  if (!bIsCarried)
    return;

  AActor *LastCarrier = CurrentCarrier;
  bIsCarried = false;
  CurrentCarrier = nullptr;

  // 부모 액터에 물리력을 가하는 등의 기본 로직
  if (AActor *OwnerActor = GetOwner()) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent())) {
      if (RootPrim->IsSimulatingPhysics()) {
        RootPrim->AddImpulse(ThrowVelocity, NAME_None, true);
      }
    }
  }

  // 로직 모듈 실행
  if (AItemBase *ItemOwner = Cast<AItemBase>(GetOwner())) {
    if (UItemData *Data = ItemOwner->GetItemData()) {
      for (ULogicModuleBase *Module : Data->LogicModules) {
        if (Module && Module->Implements<UCarryableLogicInterface>()) {
          ICarryableLogicInterface::Execute_OnModuleThrown(
              Module, LastCarrier, ItemOwner, ThrowVelocity);
        }
      }
    }
  }
}

void UCarryableComponent::SetOutlineEnabled(bool bEnabled) {
  if (AActor *OwnerActor = GetOwner()) {
    TArray<UPrimitiveComponent *> PrimitiveComps;
    OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
    for (UPrimitiveComponent *Comp : PrimitiveComps) {
      if (Comp) {
        Comp->SetRenderCustomDepth(bEnabled);
        Comp->SetCustomDepthStencilValue(1); // 외곽선을 위한 스텐실 값
      }
    }
  }
}
