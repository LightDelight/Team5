// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Trash.h"

#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Item/ContainerItemBase.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "LKH2/Logic/LogicContextInterface.h"

bool ULogic_CarryInteract_Trash::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!Interactor || !TargetActor)
    return false;

  // 1. 플레이어의 CarryComponent 획득
  UCarryComponent *CarrierComp = nullptr;
  if (Interactor->Implements<UInstigatorContextInterface>()) {
    CarrierComp =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  if (!CarrierComp)
    return false;

  AActor *PlayerActor = CarrierComp->GetCarriedActor();
  if (!PlayerActor)
    return false; // 손이 비어있으면 처리 없음

  // 2. ContainerItemBase인지 판별
  AContainerItemBase *ContainerItem =
      Cast<AContainerItemBase>(PlayerActor);

  if (ContainerItem) {
    // ── ContainerItemBase: 컨테이너 자체는 유지, 담긴 아이템만 제거 ──
    if (!TargetActor->HasAuthority())
      return true;

    // 컨테이너의 블랙보드에서 거치된 아이템들을 모두 찾아 파괴
    if (ILogicContextInterface *Context =
            Cast<ILogicContextInterface>(ContainerItem)) {
      FLogicBlackboard *Blackboard = Context->GetLogicBlackboard();
      if (Blackboard) {
        bool bRemovedAny = false;
        for (FLogicBlackboardObjectEntry &Entry :
             Blackboard->ObjectBlackboard.Items) {
          AActor *StoredActor = Cast<AActor>(Entry.ObjectValue);
          if (StoredActor) {
            StoredActor->Destroy();
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
    PlayerActor->Destroy();
    return true;
  }
}
