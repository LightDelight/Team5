// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Vending.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Data/ItemStatValue.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"

TArray<FGameplayTag>
ULogic_CarryInteract_Vending::GetRequiredStatTags() const {
  TArray<FGameplayTag> Tags;
  if (VendingItemDataTag.IsValid())
    Tags.Add(VendingItemDataTag);
  return Tags;
}

bool ULogic_CarryInteract_Vending::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!Interactor || !TargetActor)
    return false;

  // Stats에서 ItemData 조회
  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(TargetActor);
  if (!Context)
    return false;

  const FItemStatValue *DataStat = Context->FindStat(VendingItemDataTag);
  UItemData *VendingItemData =
      DataStat ? Cast<UItemData>(DataStat->ObjectValue.Get()) : nullptr;

  if (!VendingItemData)
    return false;

  // 스폰 클래스는 ItemData의 VisualPreset에서 가져옴
  TSubclassOf<AItemBase> VendingItemClass =
      VendingItemData->GetEffectiveItemClass();
  if (!VendingItemClass)
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

  UItemManagerSubsystem *ItemMgr = World->GetSubsystem<UItemManagerSubsystem>();
  if (!ItemMgr)
    return false;

  // CarrierComp 위치에 스폰 (바로 손에 쥐어질 것이므로)
  FTransform SpawnTransform = CarrierComp->GetComponentTransform();

  AItemBase *NewItem =
      ItemMgr->SpawnItemFromData(VendingItemData, SpawnTransform,
                                 VendingItemClass);
  if (!NewItem)
    return false;

  CarrierComp->ForceEquip(NewItem);

  return true;
}

void ULogic_CarryInteract_Vending::InitializeLogic(AActor *OwnerActor) {
  if (!OwnerActor || !DisplayActorKey.IsValid())
    return;

  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(OwnerActor);
  if (!Context)
    return;

  // Stats에서 데이터 조회
  const FItemStatValue *DataStat = Context->FindStat(VendingItemDataTag);
  UItemData *VendingItemData =
      DataStat ? Cast<UItemData>(DataStat->ObjectValue.Get()) : nullptr;

  if (!VendingItemData)
    return;

  // 스폰 클래스는 ItemData의 VisualPreset에서 가져옴
  TSubclassOf<AItemBase> VendingItemClass =
      VendingItemData->GetEffectiveItemClass();
  if (!VendingItemClass)
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

  UItemManagerSubsystem *ItemMgr = World->GetSubsystem<UItemManagerSubsystem>();
  if (!ItemMgr)
    return;

  FTransform DisplayTransform = SnapComp->GetComponentTransform();

  AItemBase *DisplayItem =
      ItemMgr->SpawnItemFromData(VendingItemData, DisplayTransform,
                                 VendingItemClass);
  if (!DisplayItem)
    return;

  // Manager API로 Display 아이템을 Stored 상태로 거치
  ItemMgr->StoreItem(DisplayItem, SnapComp);

  // 블랙보드에 Display 액터 등록
  Blackboard->ObjectBlackboard.SetObject(DisplayActorKey, DisplayItem);
}
