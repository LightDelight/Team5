// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Drop.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

bool ULogic_Interactable_Drop::PreInteractCheck(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  if (!TargetActor || !Context.Interactor)
    return false;

  // 1. 상호작용 유형 검사 (Interact인 경우에만 내려놓기 가능)
  if (Context.InteractionType != EInteractionType::Interact)
    return false;

  // 2. 아이템 상태 검사 (내가 손에 들려있는지 확인)
  if (Context.InHandActor != TargetActor)
    return false;

  return true;
}

bool ULogic_Interactable_Drop::PerformInteraction(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);
  if (!TargetItem)
    return false;

  UInteractorComponent *Carrier = Context.Interactor->FindComponentByClass<UInteractorComponent>();
  USceneComponent *AttachTarget = Context.Interactor->GetRootComponent();
  if (Carrier) AttachTarget = Cast<USceneComponent>(Carrier);

  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr = World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (ItemMgr) {
    // 내려놓기는 속도 0으로 드롭
    ItemMgr->DropItem(TargetItem->GetInstanceId(), FVector::ZeroVector, Carrier);
    return true;
  }

  return false;
}
