// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_WorkstationInteract_Trash.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interactables/Item/ContainerItemBase.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

bool ULogic_WorkstationInteract_Trash::PerformInteraction(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AActor *Interactor = Context.Interactor;

  if (!Interactor || !TargetActor)
    return false;

  // 1. 플레이어의 InteractorComponent 획득
  UInteractorComponent *CarrierComp = Interactor->FindComponentByClass<UInteractorComponent>();

  if (!CarrierComp)
    return false;

  AActor *PlayerActor = CarrierComp->GetCarriedActor();
  if (!PlayerActor)
    return false; // 손이 비어있으면 처리 없음

  // ItemManagerSubsystem 획득
  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr =
      World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  // 2. ContainerItemBase인지 판별
  AContainerItemBase *ContainerItem =
      Cast<AContainerItemBase>(PlayerActor);

  if (ContainerItem) {
    // ── ContainerItemBase: 컨테이너 자체는 유지, 담긴 아이템만 제거 ──
    if (!TargetActor->HasAuthority())
      return true;

    // 컨테이너의 블랙보드에서 거치된 아이템들을 모두 찾아 파괴
    if (ILogicContextInterface *LogicCtx =
            Cast<ILogicContextInterface>(ContainerItem)) {
      FLogicBlackboard *Blackboard = LogicCtx->GetLogicBlackboard();
      if (Blackboard) {
        bool bRemovedAny = false;
        for (FLogicBlackboardObjectEntry &Entry :
             Blackboard->ObjectBlackboard.Items) {
          AItemBase *StoredItem = Cast<AItemBase>(Entry.ObjectValue);
          if (StoredItem) {
            // Manager API로 파괴
            if (ItemMgr) {
              ItemMgr->DestroyItem(StoredItem->GetInstanceId());
            }
            Entry.ObjectValue = nullptr;
            Blackboard->ObjectBlackboard.MarkItemDirty(Entry);
            bRemovedAny = true;
          }
        }
        return bRemovedAny;
      }
    }

    return false;
  } else {
    // ── 일반 아이템: ForceDrop 후 파괴 ──
    if (!TargetActor->HasAuthority())
      return true;

    CarrierComp->ForceDrop();

    AItemBase *Item = Cast<AItemBase>(PlayerActor);
    if (Item && ItemMgr) {
      // Manager API로 파괴
      ItemMgr->DestroyItem(Item->GetInstanceId());
    }
    return true;
  }
}
