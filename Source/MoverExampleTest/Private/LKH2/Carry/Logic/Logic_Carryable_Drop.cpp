// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_Carryable_Drop.h"
#include "Engine/World.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"

bool ULogic_Carryable_Drop::PreInteractCheck(const FCarryContext &Context) {
  AActor *TargetActor = GetOwner();
  if (!TargetActor || !Context.Interactor)
    return false;

  // 1. 상호작용 유형 검사 (Interact인 경우에만 내려놓기 가능)
  if (Context.InteractionType != ECarryInteractionType::Interact)
    return false;

  // 2. 아이템 상태 검사 (내가 손에 들려있는지 확인)
  if (Context.InHandActor != TargetActor)
    return false;

  return true;
}

bool ULogic_Carryable_Drop::PerformInteraction(const FCarryContext &Context) {
  AActor *TargetActor = GetOwner();
  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);
  if (!TargetItem)
    return false;

  UCarryComponent *Carrier = nullptr;
  if (Context.Interactor->Implements<UInstigatorContextInterface>()) {
    Carrier = IInstigatorContextInterface::Execute_GetCarryComponent(Context.Interactor);
  }

  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr = World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (ItemMgr) {
    // 내려놓기는 속도 0으로 드롭
    ItemMgr->DropItem(TargetItem->GetInstanceId(), FVector::ZeroVector, Carrier);
    return true;
  }

  return false;
}
