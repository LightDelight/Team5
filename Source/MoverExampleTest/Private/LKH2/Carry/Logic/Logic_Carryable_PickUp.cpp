// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_Carryable_PickUp.h"
#include "Engine/World.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"

bool ULogic_Carryable_PickUp::PreInteractCheck(const FCarryContext &Context) {
  AActor *TargetActor = GetOwner();
  if (!TargetActor || !Context.Interactor)
    return false;

  // 1. 상호작용 유형 검사 (Interact인 경우에만 줍기 가능)
  if (Context.InteractionType != ECarryInteractionType::Interact)
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

bool ULogic_Carryable_PickUp::PerformInteraction(const FCarryContext &Context) {
  AActor *TargetActor = GetOwner();
  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);
  if (!TargetItem)
    return false;

  USceneComponent *AttachTarget = Context.InHandActor ? Context.InHandActor->GetRootComponent() : Context.Interactor->GetRootComponent();
  // CarryComponent를 찾을 수 있다면 그것을 우선 사용 (기존 로직 유지)
  if (Context.Interactor->Implements<UInstigatorContextInterface>()) {
    UCarryComponent *Carrier = IInstigatorContextInterface::Execute_GetCarryComponent(Context.Interactor);
    if (Carrier) AttachTarget = Cast<USceneComponent>(Carrier);
  }

  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr = World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (ItemMgr && AttachTarget) {
    ItemMgr->PickUpItem(TargetItem->GetInstanceId(), AttachTarget);
    return true;
  }

  return false;
}
