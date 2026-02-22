// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Vending.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "LKH2/Logic/LogicContextInterface.h"

bool ULogic_CarryInteract_Vending::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!Interactor || !TargetActor || !VendingItemData || !VendingItemClass)
    return false;

  // 1. 플레이어의 CarryComponent 획득
  UCarryComponent *CarrierComp = nullptr;
  if (Interactor->Implements<UInstigatorContextInterface>()) {
    CarrierComp =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  if (!CarrierComp)
    return false;

  // 2. 이미 무언가를 들고 있으면 공급 거부 (빈 손만 허용)
  if (CarrierComp->GetCarriedActor() != nullptr)
    return false;

  // 3. 아이템 스폰 및 지급 (서버 전용)
  if (!TargetActor->HasAuthority())
    return true;

  UWorld *World = TargetActor->GetWorld();
  if (!World)
    return false;

  // CarrierComp 위치에 스폰 (바로 손에 쥐어질 것이므로)
  FTransform SpawnTransform = CarrierComp->GetComponentTransform();

  AItemBase *NewItem =
      World->SpawnActorDeferred<AItemBase>(VendingItemClass, SpawnTransform);
  if (!NewItem)
    return false;

  // 아이템 데이터 적용
  NewItem->SetItemDataAndApply(VendingItemData);

  // 스폰 확정
  NewItem->FinishSpawning(SpawnTransform);

  // ForceEquip → OnCarryInteract → Logic_Carryable_Common 픽업 경로가
  // 물리 비활성화 + Carried 상태 + CarrierComp 부착을 자동 처리
  CarrierComp->ForceEquip(NewItem);

  return true;
}

void ULogic_CarryInteract_Vending::InitializeLogic(AActor *OwnerActor) {
  if (!OwnerActor || !DisplayActorKey.IsValid() || !VendingItemData ||
      !VendingItemClass)
    return;

  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(OwnerActor);
  if (!Context)
    return;

  FLogicBlackboard *Blackboard = Context->GetLogicBlackboard();
  if (!Blackboard)
    return;

  // 이미 Display가 존재하고 유효한지 확인
  AActor *ExistingDisplay =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(DisplayActorKey));
  if (ExistingDisplay && IsValid(ExistingDisplay))
    return;

  // Display용 아이템 스폰 (서버에서만 생성, 리플리케이션으로 동기화)
  if (!OwnerActor->HasAuthority())
    return;

  UCarryInteractComponent *SnapComp = Context->GetCarryInteractComponent();
  if (!SnapComp)
    return;

  UWorld *World = OwnerActor->GetWorld();
  if (!World)
    return;

  FTransform DisplayTransform = SnapComp->GetComponentTransform();

  AItemBase *DisplayItem = World->SpawnActorDeferred<AItemBase>(
      VendingItemClass, DisplayTransform);
  if (!DisplayItem)
    return;

  // 데이터 적용 (메쉬 등 시각 요소 세팅)
  DisplayItem->SetItemDataAndApply(VendingItemData);

  // 물리/충돌 완전 비활성화 (순수 시각용)
  if (UPrimitiveComponent *RootPrim =
          Cast<UPrimitiveComponent>(DisplayItem->GetRootComponent())) {
    RootPrim->SetSimulatePhysics(false);
    RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
  }

  // Stored 상태로 설정 (물리가 꺼진 채로 부착 유지)
  if (UItemStateComponent *StateComp =
          DisplayItem->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Stored);
  }

  // 스폰 확정 후 CarryInteractComponent에 부착
  DisplayItem->FinishSpawning(DisplayTransform);
  DisplayItem->AttachToComponent(
      SnapComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

  // 블랙보드에 Display 액터 등록
  Blackboard->ObjectBlackboard.SetObject(DisplayActorKey, DisplayItem);
}

