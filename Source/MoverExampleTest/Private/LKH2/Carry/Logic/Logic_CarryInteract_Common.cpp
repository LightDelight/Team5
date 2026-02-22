// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Common.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h" // 플레이어의 Carry Component (Item을 들고 있는지 확인 용도)
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicContextInterface.h"

#include "LKH2/Logic/LogicBlackboard.h"

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

  if (CurrentStoredItem == nullptr) {
    // 워크스테이션이 비어있음
    if (PlayerItem != nullptr) {
      if (TargetActor->HasAuthority()) {
        // 플레이어 손에서 분리하고 워크스테이션에 부착 (놓기)
        CarrierComp->ForceDrop(); // 아이템 자체의 OnDropped 이 불림

        if (UPrimitiveComponent *RootPrim =
                Cast<UPrimitiveComponent>(PlayerItem->GetRootComponent())) {
          RootPrim->SetSimulatePhysics(false);
          RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
        }

        if (UItemStateComponent *StateComp =
                PlayerItem->FindComponentByClass<UItemStateComponent>()) {
          StateComp->SetItemState(EItemState::Stored);
        }

        UCarryInteractComponent *SnapComp = nullptr;
        if (ILogicContextInterface *Context =
                Cast<ILogicContextInterface>(TargetActor)) {
          SnapComp = Context->GetCarryInteractComponent();
        } else {
          SnapComp =
              TargetActor->FindComponentByClass<UCarryInteractComponent>();
        }

        if (SnapComp) {
          PlayerItem->AttachToComponent(
              SnapComp,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        } else {
          PlayerItem->AttachToActor(
              TargetActor,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }

        // 블랙보드에 배치된 아이템 상태 업데이트 (자동 멀티플레이 복제됨)
        Blackboard->ObjectBlackboard.SetObject(StoredItemKey, PlayerItem);
      }
      return true;
    }
  } else {
    // 워크스테이션에 이미 아이템이 있음
    if (PlayerItem == nullptr) {
      if (TargetActor->HasAuthority()) {
        // 플레이어 손이 비어있음 -> 아이템을 플레이어에게 전달 (줍기)
        CurrentStoredItem->DetachFromActor(
            FDetachmentTransformRules::KeepWorldTransform);
        CarrierComp->ForceEquip(CurrentStoredItem);

        if (UItemStateComponent *StateComp =
                CurrentStoredItem
                    ->FindComponentByClass<UItemStateComponent>()) {
          StateComp->SetItemState(EItemState::Carried);
        }

        // 블랙보드에서 거치된 아이템 초기화
        Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
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
