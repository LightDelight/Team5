// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_PickUp.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

bool ULogic_Interactable_PickUp::PreInteractCheck(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  if (!TargetActor || !Context.Interactor)
    return false;

  // 1. 상호작용 유형 검사 (Interact인 경우에만 줍기 가능)
  if (Context.InteractionType != EInteractionType::Interact)
    return false;

  // 2. 아이템 상태 검사 (이미 들려있으면 줍기 불가)
  UItemStateComponent *StateComp = TargetActor->FindComponentByClass<UItemStateComponent>();
  if (!StateComp || StateComp->CurrentState == EItemState::Carried)
    return false;

  // 3. 인터랙터의 소지 상태 확인 (이미 무언가를 들고 있으면 줍기 불가)
  if (Context.bIsHandOccupied) {
    return false;
  }

  return true;
}

bool ULogic_Interactable_PickUp::PerformInteraction(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);
  if (!TargetItem)
    return false;

  USceneComponent *AttachTarget = Context.InHandActor ? Context.InHandActor->GetRootComponent() : Context.Interactor->GetRootComponent();
  UInteractorComponent *Carrier = Context.Interactor->FindComponentByClass<UInteractorComponent>();
  if (Carrier) AttachTarget = Cast<USceneComponent>(Carrier);

  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr = World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (ItemMgr && AttachTarget) {
    ItemMgr->PickUpItem(TargetItem->GetInstanceId(), AttachTarget, Carrier);
    return true;
  }

  return false;
}
