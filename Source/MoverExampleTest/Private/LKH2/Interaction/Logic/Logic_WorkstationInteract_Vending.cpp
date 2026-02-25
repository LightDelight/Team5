// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_WorkstationInteract_Vending.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

TArray<FGameplayTag>
ULogic_WorkstationInteract_Vending::GetRequiredStatTags() const {
  TArray<FGameplayTag> Tags;
  if (VendingItemTag.IsValid())
    Tags.Add(VendingItemTag);
  return Tags;
}

bool ULogic_WorkstationInteract_Vending::PreInteractCheck(const FInteractionContext &Context) {
  AActor *Interactor = Context.Interactor;
  AActor *TargetActor = GetOwner();

  if (!Interactor || !TargetActor)
    return false;

  // 1. 플레이어의 InteractorComponent 획득
  UInteractorComponent *CarrierComp = Interactor->FindComponentByClass<UInteractorComponent>();

  if (!CarrierComp)
    return false;

  // 2. 이미 무언가를 들고 있으면 공급 거부 (빈 손만 허용)
  if (CarrierComp->GetCarriedActor() != nullptr)
    return false;

  return true;
}

bool ULogic_WorkstationInteract_Vending::PerformInteraction(const FInteractionContext &Context) {
  AActor *Interactor = Context.Interactor;
  AActor *TargetActor = GetOwner();

  // Stats에서 ItemData 조회
  ILogicContextInterface *LogicCtx =
      Cast<ILogicContextInterface>(TargetActor);
  if (!LogicCtx)
    return false;

  const FItemStatValue *DataStat = LogicCtx->FindStat(VendingItemTag);
  FGameplayTag ItemTag =
      DataStat ? DataStat->TagValue : FGameplayTag::EmptyTag;

  if (!ItemTag.IsValid())
    return false;

  UInteractorComponent *CarrierComp =
      Interactor->FindComponentByClass<UInteractorComponent>();

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

  FGuid NewInstanceId = ItemMgr->SpawnItem(ItemTag, SpawnTransform);

  // RetrieveItem을 사용해 상태 전이(Carried)와 장착을 동시에 처리
  ItemMgr->RetrieveItem(NewInstanceId, CarrierComp);

  return true;
}

void ULogic_WorkstationInteract_Vending::InitializeLogic(AActor *InOwnerActor) {
  Super::InitializeLogic(InOwnerActor);
  if (!InOwnerActor || !DisplayActorKey.IsValid())
    return;

  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(InOwnerActor);
  if (!Context)
    return;

  // Stats에서 데이터 조회
  const FItemStatValue *DataStat = Context->FindStat(VendingItemTag);
  FGameplayTag ItemTag =
      DataStat ? DataStat->TagValue : FGameplayTag::EmptyTag;

  if (!ItemTag.IsValid())
    return;

  FLogicBlackboard *Blackboard = Context->GetLogicBlackboard();
  if (!Blackboard)
    return;

  FGameplayTag ActualDisplayKey = Context->ResolveKey(DisplayActorKey);
  if (!ActualDisplayKey.IsValid())
    return;

  // 이미 Display가 존재하고 유효한지 확인
  AActor *ExistingDisplay =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(ActualDisplayKey));
  if (ExistingDisplay && IsValid(ExistingDisplay))
    return;

  // Display용 아이템 스폰 (서버에서만 생성, 리플리케이션으로 동기화)
  if (!OwnerActor->HasAuthority())
    return;

  UInteractableComponent *SnapComp = Context->GetInteractableComponent();
  if (!SnapComp)
    return;

  UWorld *World = OwnerActor->GetWorld();
  if (!World)
    return;

  UItemManagerSubsystem *ItemMgr = World->GetSubsystem<UItemManagerSubsystem>();
  if (!ItemMgr)
    return;

  FTransform DisplayTransform = SnapComp->GetComponentTransform();

  FGuid DisplayInstanceId = ItemMgr->SpawnItem(ItemTag, DisplayTransform);
  AItemBase *DisplayItem = ItemMgr->GetItemActor(DisplayInstanceId);
  if (!DisplayItem)
    return;

  // Manager API로 Display 아이템을 Stored 상태로 거치
  ItemMgr->StoreItem(DisplayInstanceId, SnapComp);

  // 블랙보드에 Display 액터 등록
  Blackboard->ObjectBlackboard.SetObject(ActualDisplayKey, DisplayItem);
}
