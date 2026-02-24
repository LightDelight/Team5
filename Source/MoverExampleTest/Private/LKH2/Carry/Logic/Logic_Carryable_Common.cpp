// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_Carryable_Common.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"

bool ULogic_Carryable_Common::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!TargetActor || !Interactor)
    return false;

  UCarryComponent *Carrier = nullptr;
  if (Interactor->Implements<UInstigatorContextInterface>()) {
    Carrier =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  UItemStateComponent *StateComp =
      TargetActor->FindComponentByClass<UItemStateComponent>();
  if (!StateComp)
    return false;

  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);

  bool bIsCarried = (StateComp->CurrentState == EItemState::Carried);

  if (!bIsCarried) {
    // ----------------------------------------------------
    // 줍기 (Pick Up: Placed/Dropped/Stored -> Carried)
    // ----------------------------------------------------
    if (Carrier && Carrier->GetCarriedActor() != nullptr) {
      return false; // 이미 다른 것을 들고 있으면 실패
    }

    USceneComponent *AttachTarget =
        Carrier ? Cast<USceneComponent>(Carrier)
                : Interactor->GetRootComponent();

    UWorld *World = TargetActor->GetWorld();
    UItemManagerSubsystem *ItemMgr =
        World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

    if (ItemMgr && TargetItem && AttachTarget) {
      ItemMgr->PickUpItem(TargetItem, AttachTarget);
    }
    return true;
  } else {
    // ----------------------------------------------------
    // 내려놓기 또는 던지기 (Drop/Throw: Carried -> Placed)
    // ----------------------------------------------------
    UWorld *World = TargetActor->GetWorld();
    UItemManagerSubsystem *ItemMgr =
        World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

    if (ItemMgr && TargetItem) {
      // 던지기 판단
      FVector Impulse = FVector::ZeroVector;
      if (InteractionType == ECarryInteractionType::Throw) {
        Impulse = Interactor->GetActorForwardVector() * 800.0f +
                  FVector(0, 0, 300.0f);
      }

      ItemMgr->DropItem(TargetItem, Impulse, Carrier);
    }
    return true;
  }
}
