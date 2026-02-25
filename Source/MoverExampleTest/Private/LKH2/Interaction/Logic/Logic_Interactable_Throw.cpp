// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Throw.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

bool ULogic_Interactable_Throw::PreInteractCheck(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  if (!TargetActor || !Context.Interactor)
    return false;

  // 1. 상호작용 유형 검사 (Throw인 경우에만 던지기 가능)
  if (Context.InteractionType != EInteractionType::Throw)
    return false;

  // 2. 아이템 상태 검사 (내가 손에 들려있는지 확인)
  if (Context.InHandActor != TargetActor)
    return false;

  return true;
}

bool ULogic_Interactable_Throw::PerformInteraction(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AItemBase *TargetItem = Cast<AItemBase>(TargetActor);
  if (!TargetItem)
    return false;

  UInteractorComponent *Carrier = Context.Interactor->FindComponentByClass<UInteractorComponent>();
  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr = World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (ItemMgr) {
    // 컨텍스트에 이미 계산된 속도가 있다면 그것을 사용 (InteractorComponent에서 생성됨)
    FVector Impulse = Context.Velocity;
    
    // 만약 컨텍스트에 속도가 비어있다면(Zero) 기본값 계산
    if (Impulse.IsNearlyZero()) {
      Impulse = Context.Interactor->GetActorForwardVector() * 800.0f + FVector(0, 0, 300.0f);
    }

    ItemMgr->DropItem(TargetItem->GetInstanceId(), Impulse, Carrier);
    return true;
  }

  return false;
}
