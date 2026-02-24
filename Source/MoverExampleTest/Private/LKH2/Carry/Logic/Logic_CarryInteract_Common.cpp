// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Common.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"

ULogic_CarryInteract_Common::ULogic_CarryInteract_Common() {
  StoredItemKey = FGameplayTag::EmptyTag;
}

bool ULogic_CarryInteract_Common::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!TargetActor || !Interactor)
    return false;

  // 블랙보드 찾기
  FLogicBlackboard *Blackboard = nullptr;
  if (ILogicContextInterface *Context =
          Cast<ILogicContextInterface>(TargetActor)) {
    Blackboard = Context->GetLogicBlackboard();
  }

  if (!Blackboard || !StoredItemKey.IsValid()) {
    // 블랙보드가 없거나, 기획자가 에디터에서 키 세팅을 해주지 않았다면 로직
    // 중단
    return false;
  }

  // Interactor(플레이어)가 UCarryComponent 같은 걸 통해 아이템을 들고 있는지
  // 확인
  UCarryComponent *CarrierComp = nullptr;
  if (Interactor && Interactor->Implements<UInstigatorContextInterface>()) {
    CarrierComp =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  if (!CarrierComp)
    return false;

  AActor *PlayerItem = CarrierComp->GetCarriedActor();

  // 블랙보드에서 현재 거치된 아이템 가져오기
  AActor *CurrentStoredItem =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(StoredItemKey));

  // 유효성 검사: 누군가 직접 아이템을 주워가서 부착이 해제되었을 경우 블랙보드
  // 임의 초기화 (루즈 커플링 방어)
  if (CurrentStoredItem &&
      CurrentStoredItem->GetAttachParentActor() != TargetActor) {
    if (TargetActor->HasAuthority()) {
      Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
    }
    CurrentStoredItem = nullptr;
  }

  // ItemManagerSubsystem 획득
  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr =
      World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  // SnapComp 결정
  UCarryInteractComponent *SnapComp = nullptr;
  if (ILogicContextInterface *Context =
          Cast<ILogicContextInterface>(TargetActor)) {
    SnapComp = Context->GetCarryInteractComponent();
  } else {
    SnapComp =
        TargetActor->FindComponentByClass<UCarryInteractComponent>();
  }

  // 부착 대상 결정 (SnapComp가 있으면 SnapComp, 없으면 TargetActor 루트)
  USceneComponent *AttachTarget =
      SnapComp ? Cast<USceneComponent>(SnapComp)
               : TargetActor->GetRootComponent();

  if (CurrentStoredItem == nullptr) {
    // 워크스테이션이 비어있음
    if (PlayerItem != nullptr) {
      if (ItemMgr && AttachTarget) {
        AItemBase *Item = Cast<AItemBase>(PlayerItem);
        if (Item) {
          // Manager API로 거치 수행 (클라이언트 예측 포함)
          ItemMgr->StoreItem(Item, AttachTarget, CarrierComp);

          // 블랙보드 상태 업데이트는 서버에서만 수행
          if (TargetActor->HasAuthority()) {
            Blackboard->ObjectBlackboard.SetObject(StoredItemKey, PlayerItem);
          }
        }
      }
      return true;
    }
  } else {
    // 워크스테이션에 이미 아이템이 있음
    if (PlayerItem == nullptr) {
      if (ItemMgr) {
        AItemBase *StoredItem = Cast<AItemBase>(CurrentStoredItem);
        if (StoredItem) {
          // Manager API로 회수 수행 (클라이언트 예측 포함)
          ItemMgr->RetrieveItem(StoredItem, CarrierComp);

          // 블랙보드 초기화는 서버에서만 수행
          if (TargetActor->HasAuthority()) {
            Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
          }
        }
      }
      return true;
    } else {
      // 플레이어도 들고 있고, 워크스테이션도 있음 -> 교체할지, 아무 동작 안
      // 할지 결정 (기본은 무시)
      return false;
    }
  }
  return false;
}
