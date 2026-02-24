// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Combine.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Data/ItemStatValue.h"
#include "LKH2/Item/ContainerItemBase.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Item/ItemStateComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "LKH2/Manager/ItemManagerSubsystem.h"
#include "LKH2/Recipe/CombineRecipeBook.h"

ULogic_CarryInteract_Combine::ULogic_CarryInteract_Combine() {
  StoredItemKey = FGameplayTag::EmptyTag;
}

TArray<FGameplayTag>
ULogic_CarryInteract_Combine::GetRequiredStatTags() const {
  TArray<FGameplayTag> Tags;
  if (RecipeBooksTag.IsValid())
    Tags.Add(RecipeBooksTag);
  return Tags;
}

void ULogic_CarryInteract_Combine::InitializeLogic(AActor *OwnerActor) {
  if (!OwnerActor)
    return;

  // Stats에서 레시피 책 배열 조회
  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(OwnerActor);
  if (!Context)
    return;

  // 레시피 캐싱
  if (const FItemStatValue *BooksStat = Context->FindStat(RecipeBooksTag)) {
    if (BooksStat->Type == EItemStatType::ObjectArray) {
      CachedRecipes.Empty();
      for (const TObjectPtr<UObject> &Obj : BooksStat->ObjectArrayValue) {
        UCombineRecipeBook *Book = Cast<UCombineRecipeBook>(Obj.Get());
        if (Book) {
          for (const FCombineRecipe &Recipe : Book->Recipes) {
            CachedRecipes.Add(Recipe);
          }
        }
      }

      // 부모 클래스 템플릿 정렬
      SortRecipes(CachedRecipes,
                  [](const FCombineRecipe &A, const FCombineRecipe &B) {
                    return A.Priority < B.Priority;
                  });
    }
  }
}

void ULogic_CarryInteract_Combine::CacheRecipes() {
  Super::CacheRecipes();

  // InitializeLogic에서 Stats 기반으로 캐싱하므로
  // PostLoad/PostEditChangeProperty 시에는 빈 상태로 대기
}

bool ULogic_CarryInteract_Combine::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!TargetActor || !Interactor)
    return false;

  FLogicBlackboard *Blackboard = nullptr;
  if (ILogicContextInterface *Context =
          Cast<ILogicContextInterface>(TargetActor)) {
    Blackboard = Context->GetLogicBlackboard();
  }

  if (!Blackboard || !StoredItemKey.IsValid())
    return false;

  UCarryComponent *CarrierComp = nullptr;
  if (Interactor && Interactor->Implements<UInstigatorContextInterface>()) {
    CarrierComp =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  if (!CarrierComp)
    return false;

  AActor *PlayerActor = CarrierComp->GetCarriedActor();
  AItemBase *PlayerItem = Cast<AItemBase>(PlayerActor);

  AActor *StoredActor =
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(StoredItemKey));

  // 유효성 검사: 누군가 직접 줍기 상호작용으로 아이템을 떼어갔다면 블랙보드
  // 클리어
  if (StoredActor && StoredActor->GetAttachParentActor() != TargetActor) {
    if (TargetActor->HasAuthority()) {
      Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
    }
    StoredActor = nullptr;
  }

  AItemBase *StoredItem = Cast<AItemBase>(StoredActor);

  // SnapComp / AttachTarget 결정
  UCarryInteractComponent *SnapComp = nullptr;
  if (ILogicContextInterface *Context =
          Cast<ILogicContextInterface>(TargetActor)) {
    SnapComp = Context->GetCarryInteractComponent();
  } else {
    SnapComp = TargetActor->FindComponentByClass<UCarryInteractComponent>();
  }

  USceneComponent *AttachTarget =
      SnapComp ? Cast<USceneComponent>(SnapComp)
               : TargetActor->GetRootComponent();

  // ItemManagerSubsystem 획득
  UWorld *World = TargetActor->GetWorld();
  UItemManagerSubsystem *ItemMgr =
      World ? World->GetSubsystem<UItemManagerSubsystem>() : nullptr;

  if (StoredActor == nullptr) {
    // 워크스테이션이 비어있음 -> 거치 수행
    if (PlayerActor != nullptr) {
      // ContainerItemBase(접시 등)는 수납 대상에서 제외
      if (Cast<AContainerItemBase>(PlayerActor)) {
        return false;
      }
      if (ItemMgr && AttachTarget) {
        // Manager API로 거치 수행 (클라이언트 예측 포함)
        ItemMgr->StoreItem(PlayerItem, AttachTarget, CarrierComp);

        // 블루프린트나 서버 측 로직을 위한 블랙보드 상태 업데이트는 서버 전용
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(StoredItemKey, PlayerActor);
        }
      }
      return true;
    }
  } else {
    // 워크스테이션에 아이템이 거치되어 있음
    if (PlayerActor == nullptr) {
      // 플레이어 손이 비어있음 -> 회수 수행
      if (ItemMgr && StoredItem) {
        // Manager API로 회수 수행 (클라이언트 예측 포함)
        ItemMgr->RetrieveItem(StoredItem, CarrierComp);

        // 블루프린트나 서버 측 로직을 위한 블랙보드 상태 업데이트는 서버 전용
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
        }
      }
      return true;
    } else if (PlayerItem && StoredItem) {
      // 플레이어 손에도 아이템이 있음 -> 조합(Combine) 시도
      UItemData *PlayerData = PlayerItem->GetItemData();
      UItemData *StoredData = StoredItem->GetItemData();

      if (PlayerData && StoredData) {
        const FCombineRecipe *FoundRecipe = nullptr;
        if (TryFindRecipe(StoredData, PlayerData, FoundRecipe) &&
            FoundRecipe != nullptr) {
          // 레시피 매칭 성공 — 클래스는 ResultItemData에서 가져옴
          TSubclassOf<AItemBase> ResultClass =
              FoundRecipe->ResultItemData
                  ? FoundRecipe->ResultItemData->GetEffectiveItemClass()
                  : nullptr;

          if (TargetActor->HasAuthority() && ItemMgr && ResultClass) {
            CarrierComp->ForceDrop(); // 손에서 내려놓기 처리

            FTransform SpawnTransform =
                SnapComp ? SnapComp->GetComponentTransform()
                         : TargetActor->GetActorTransform();

            // 기존 아이템 파괴 (ItemManager 추적)
            ItemMgr->DestroyItem(PlayerItem);
            ItemMgr->DestroyItem(StoredItem);

            // 결과 아이템 스폰 (ItemManager 파이프라인)
            AItemBase *NewItem = ItemMgr->SpawnItemFromData(
                FoundRecipe->ResultItemData, SpawnTransform, ResultClass);

            if (NewItem && AttachTarget) {
              // Manager API로 결과 아이템 거치
              ItemMgr->StoreItem(NewItem, AttachTarget);

              // 블랙보드에 신규 거치 아이템 등록
              Blackboard->ObjectBlackboard.SetObject(StoredItemKey, NewItem);
            }
          }
          return true;
        }
      }
    }
  }

  return false;
}

bool ULogic_CarryInteract_Combine::TryFindRecipe(
    UItemData *InItemA, UItemData *InItemB,
    const FCombineRecipe *&OutRecipe) const {
  for (const FCombineRecipe &Recipe : CachedRecipes) {
    if (!Recipe.MaterialA || !Recipe.MaterialB || !Recipe.ResultItemData)
      continue;

    // 1. 매칭: 재료 A = 거치아이템, 재료 B = 플레이어아이템
    if (Recipe.MaterialA == InItemA && Recipe.MaterialB == InItemB) {
      OutRecipe = &Recipe;
      return true;
    }

    // 2. 순서 무관 매칭
    if (Recipe.MaterialA == InItemB && Recipe.MaterialB == InItemA) {
      OutRecipe = &Recipe;
      return true;
    }
  }
  return false;
}
