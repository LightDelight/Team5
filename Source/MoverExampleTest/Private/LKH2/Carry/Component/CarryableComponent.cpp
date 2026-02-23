// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Component/CarryableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Logic/Interface/CarryLogicInterface.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Logic/LogicModuleBase.h"

UCarryableComponent::UCarryableComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  bIsCarried = false;
  CurrentCarrier = nullptr;
}

void UCarryableComponent::BeginPlay() { Super::BeginPlay(); }

bool UCarryableComponent::OnCarryInteract(
    AActor *Interactor, ECarryInteractionType InteractionType) {
  if (GetOwner()) {
    if (AItemBase *Item = Cast<AItemBase>(GetOwner())) {
      if (UItemData *ItemData = Item->GetItemData()) {
        const TArray<ULogicModuleBase *> Modules =
            ItemData->GetAllModules();

        // 책임 연쇄 패턴(Chain of Responsibility)으로 순회
        for (ULogicModuleBase *Module : Modules) {
          if (Module && Module->Implements<UCarryLogicInterface>()) {
            if (ICarryLogicInterface::Execute_OnModuleInteract(
                    Module, Interactor, GetOwner(), InteractionType)) {
              return true; // 로직 중단
            }
          }
        }
      }
    }
  }
  return false;
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
