// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Logic_CarryInteract_Common.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/CarryComponent.h" // 플레이어의 Carry Component (Item을 들고 있는지 확인 용도)
#include "LKH2/CarryableInterface.h"

#include "Net/UnrealNetwork.h"

ULogic_CarryInteract_Common::ULogic_CarryInteract_Common() {
  StoredItem = nullptr;
}

void ULogic_CarryInteract_Common::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(ULogic_CarryInteract_Common, StoredItem);
}

void ULogic_CarryInteract_Common::OnRep_StoredItem() {
  // 클라이언트: StoredItem의 변경 상태 동기화
}

void ULogic_CarryInteract_Common::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetWorkstation) {
  if (!Interactor || !TargetWorkstation)
    return;

  // Interactor(플레이어)가 UCarryComponent 같은 걸 통해 아이템을 들고 있는지
  // 확인 이 부분은 프로젝트 구조에 따라 UCarryComponent를 가져오는 방식이
  // 달라질 수 있습니다.
  UCarryComponent *CarrierComp =
      Interactor->FindComponentByClass<UCarryComponent>();
  if (!CarrierComp)
    return;

  // 참고: CarryComponent 의 내부 CarriedActor 에 접근하기 위해 getter 가 필요할
  // 수도 있으나 여기서는 로직 흐름 상 워크스테이션과 플레이어 간의 아이템
  // 교환을 추상화합니다.

  // (실제 프로젝트에서는 CarrierComp->GetCarriedActor() 같은 함수를 만들어
  // 사용하는 것을 권장합니다.) 임시 시나리오:
  // 1. 플레이어가 무언가 들고 있다면 거치하고 (StoredItem = 들고있는 아이템)
  // 2. 플레이어 손이 비어있다면, 현재 StoredItem을 플레이어에게 건네줍니다.

  AActor *PlayerItem = CarrierComp->GetCarriedActor();

  if (StoredItem == nullptr) {
    // 워크스테이션이 비어있음
    if (PlayerItem != nullptr) {
      // 플레이어 손에서 분리하고 워크스테이션에 부착 (놓기)
      CarrierComp->ForceDrop(); // 아이템 자체의 OnDropped 이 불림

      if (UPrimitiveComponent *RootPrim =
              Cast<UPrimitiveComponent>(PlayerItem->GetRootComponent())) {
        RootPrim->SetSimulatePhysics(false);
      }

      if (UCarryInteractComponent *SnapComp =
              TargetWorkstation
                  ->FindComponentByClass<UCarryInteractComponent>()) {
        PlayerItem->AttachToComponent(
            SnapComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
      } else {
        PlayerItem->AttachToActor(
            TargetWorkstation,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
      }

      StoredItem = PlayerItem;
    }
  } else {
    // 워크스테이션에 이미 아이템이 있음
    if (PlayerItem == nullptr) {
      // 플레이어 손이 비어있음 -> 아이템을 플레이어에게 전달 (줍기)
      StoredItem->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
      CarrierComp->ForceEquip(StoredItem);

      StoredItem = nullptr;
    } else {
      // 플레이어도 들고 있고, 워크스테이션도 있음 -> 교체할지, 아무 동작 안
      // 할지 결정 (기본은 무시)
    }
  }
}
