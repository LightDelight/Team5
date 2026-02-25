// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_WorkstationInteract_Common.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

ULogic_WorkstationInteract_Common::ULogic_WorkstationInteract_Common() {
  StoredItemKey = FGameplayTag::EmptyTag;
}

bool ULogic_WorkstationInteract_Common::PreInteractCheck(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AActor *Interactor = Context.Interactor;

  if (!TargetActor || !Interactor)
    return false;

  // Interactor(플레이어)가 UInteractorComponent 같은 걸 통해 아이템을 들고 있는지
  // 확인
  UInteractorComponent *CarrierComp = Interactor->FindComponentByClass<UInteractorComponent>();

  if (!CarrierComp)
    return false;

  AActor *PlayerItem = CarrierComp->GetCarriedActor();

  // 블랙보드 찾기
  FLogicBlackboard *Blackboard = nullptr;
  FGameplayTag ActualStoredItemKey = StoredItemKey;

  if (ILogicContextInterface *LogicCtx =
          Cast<ILogicContextInterface>(TargetActor)) {
    Blackboard = LogicCtx->GetLogicBlackboard();
    if (StoredItemKey.IsValid()) {
      ActualStoredItemKey = LogicCtx->ResolveKey(StoredItemKey);
    }
  }

  if (!Blackboard || !ActualStoredItemKey.IsValid()) {
    return false;
  }

  // 블랙보드에서 현재 거치된 아이템 가져오기
  AActor *CurrentStoredItem =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(ActualStoredItemKey));

  // 1. 거치(Store) 시도 판정
  if (CurrentStoredItem == nullptr && PlayerItem != nullptr) {
    // 거치가 금지된 경우 (휴대용 접시 등) 거부
    if (!bAllowStore)
      return false;
  }

  // 2. 회수(Retrieve) 시도 판정
  if (CurrentStoredItem != nullptr && PlayerItem == nullptr) {
    // 회수가 금지된 경우 (휴대용 접시 줍기를 유도하기 위해) 거부
    if (!bAllowRetrieve)
      return false;
  }

  return true;
}

bool ULogic_WorkstationInteract_Common::PerformInteraction(const FInteractionContext &Context) {
  AActor *TargetActor = GetOwner();
  AActor *Interactor = Context.Interactor;

  // [Type Filtering] 일반 상호작용(Interact)이 아닌 경우(예: Throw, Drop)는
  // 로직 모듈이 반응하지 않도록 차단합니다.
  if (Context.InteractionType != EInteractionType::Interact) {
    return false;
  }

  // PreInteractCheck를 통과했으므로 유효성 재검증은 최소화
  FLogicBlackboard *Blackboard = nullptr;
  FGameplayTag ActualStoredItemKey = StoredItemKey;

  if (ILogicContextInterface *LogicCtx =
          Cast<ILogicContextInterface>(TargetActor)) {
    Blackboard = LogicCtx->GetLogicBlackboard();
    if (StoredItemKey.IsValid()) {
      ActualStoredItemKey = LogicCtx->ResolveKey(StoredItemKey);
    }
  }

  if (!Blackboard || !ActualStoredItemKey.IsValid())
    return false;

  UInteractorComponent *CarrierComp = Interactor->FindComponentByClass<UInteractorComponent>();
  AActor *PlayerItem = CarrierComp->GetCarriedActor();

  AActor *CurrentStoredItem =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(ActualStoredItemKey));

  // 유효성 검사: 누군가 직접 아이템을 주워가서 부착이 해제되었을 경우 블랙보드
  // 임의 초기화 (루즈 커플링 방어)
  if (CurrentStoredItem &&
      CurrentStoredItem->GetAttachParentActor() != TargetActor) {
    if (TargetActor->HasAuthority()) {
      Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, nullptr);
    }
    CurrentStoredItem = nullptr;
  }

  // ItemManagerSubsystem 획득
  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr =
      World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  // SnapComp 결정
  UInteractableComponent *SnapComp = nullptr;
  if (ILogicContextInterface *LogicCtx =
          Cast<ILogicContextInterface>(TargetActor)) {
    SnapComp = LogicCtx->GetInteractableComponent();
  }

  // 부착 대상 결정 (SnapComp가 있으면 SnapComp, 없으면 TargetActor 루트)
  USceneComponent *AttachTarget =
      SnapComp ? Cast<USceneComponent>(SnapComp)
               : TargetActor->GetRootComponent();

  if (CurrentStoredItem == nullptr) {
    // 워크스테이션이 비어있음 -> 거치 (Store)
    if (PlayerItem != nullptr && ItemMgr && AttachTarget) {
      AItemBase *Item = Cast<AItemBase>(PlayerItem);
      if (Item) {
        ItemMgr->StoreItem(Item->GetInstanceId(), AttachTarget, CarrierComp);
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, PlayerItem);
        }
        return true;
      }
    }
  } else {
    // 워크스테이션에 이미 아이템이 있음 -> 회수 (Retrieve)
    if (PlayerItem == nullptr && ItemMgr) {
      AItemBase *StoredItem = Cast<AItemBase>(CurrentStoredItem);
      if (StoredItem) {
        ItemMgr->RetrieveItem(StoredItem->GetInstanceId(), CarrierComp);
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, nullptr);
        }
        return true;
      }
    }
  }
  return false;
}
