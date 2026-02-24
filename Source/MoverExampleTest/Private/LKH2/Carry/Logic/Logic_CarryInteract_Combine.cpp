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

void ULogic_CarryInteract_Combine::InitializeLogic(AActor *InOwnerActor) {
  Super::InitializeLogic(InOwnerActor);
  if (!InOwnerActor)
    return;

  // Stats에서 레시피 책 배열 조회
  ILogicContextInterface *Context =
      Cast<ILogicContextInterface>(InOwnerActor);
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

bool ULogic_CarryInteract_Combine::PerformInteraction(const FCarryContext &Context) {
  AActor *TargetActor = GetOwner();
  AActor *Interactor = Context.Interactor;

  if (!TargetActor || !Interactor)
    return false;

  // [Type Filtering] 일반 상호작용(Interact)이 아닌 경우(예: Throw, Drop)는
  // 로직 모듈이 반응하지 않도록 차단합니다.
  if (Context.InteractionType != ECarryInteractionType::Interact) {
    return false;
  }

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
      Cast<AActor>(Blackboard->ObjectBlackboard.GetObject(ActualStoredItemKey));

  // 유효성 검사: 누군가 직접 줍기 상호작용으로 아이템을 떼어갔다면 블랙보드 클리어
  if (StoredActor && StoredActor->GetAttachParentActor() != TargetActor) {
    if (TargetActor->HasAuthority()) {
      Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, nullptr);
    }
    StoredActor = nullptr;
  }

  AItemBase *StoredItem = Cast<AItemBase>(StoredActor);

  // SnapComp / AttachTarget 결정
  UCarryInteractComponent *SnapComp = nullptr;
  if (ILogicContextInterface *LogicCtx =
          Cast<ILogicContextInterface>(TargetActor)) {
    SnapComp = LogicCtx->GetCarryInteractComponent();
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
        ItemMgr->StoreItem(PlayerItem->GetInstanceId(), AttachTarget, CarrierComp);

        // 블랙보드 상태 업데이트는 서버 전용
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, PlayerActor);
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
        ItemMgr->RetrieveItem(StoredItem->GetInstanceId(), CarrierComp);

        // 블랙보드 상태 업데이트는 서버 전용
        if (TargetActor->HasAuthority()) {
          Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, nullptr);
        }
      }
      return true;
    } else if (PlayerItem && StoredItem) {
      // 플레이어 손에도 아이템이 있음 -> 조합(Combine) 시도
      UItemData *PlayerData = PlayerItem->GetItemData();
      UItemData *StoredData = StoredItem->GetItemData();

      if (PlayerData && StoredData) {
        const FCombineRecipe *FoundRecipe = nullptr;
        if (TryFindRecipe(StoredData->ItemTag, PlayerData->ItemTag, FoundRecipe) &&
            FoundRecipe != nullptr) {
          // 레시피 매칭 성공
          if (TargetActor->HasAuthority() && ItemMgr) {
            CarrierComp->ForceDrop(); // 손에서 내려놓기 처리

            FTransform SpawnTransform =
                SnapComp ? SnapComp->GetComponentTransform()
                         : TargetActor->GetActorTransform();

            // 기존 아이템 파괴 (ItemManager 추적)
            ItemMgr->DestroyItem(PlayerItem->GetInstanceId());
            ItemMgr->DestroyItem(StoredItem->GetInstanceId());

            // 결과 아이템 태그 기반 스폰 (ItemManager 파이프라인)
            FGuid NewInstanceId =
                ItemMgr->SpawnItem(FoundRecipe->ResultItemTag, SpawnTransform);
            AItemBase *NewItem = ItemMgr->GetItemActor(NewInstanceId);

            if (NewItem && AttachTarget) {
              // Manager API로 결과 아이템 거치
              ItemMgr->StoreItem(NewInstanceId, AttachTarget);

              // 블랙보드에 신규 거치 아이템 등록
              Blackboard->ObjectBlackboard.SetObject(ActualStoredItemKey, NewItem);
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
    FGameplayTag InTagA, FGameplayTag InTagB,
    const FCombineRecipe *&OutRecipe) const {
  for (const FCombineRecipe &Recipe : CachedRecipes) {
    if (!Recipe.MaterialA.IsValid() || !Recipe.MaterialB.IsValid() ||
        !Recipe.ResultItemTag.IsValid())
      continue;

    // 1. 매칭: 재료 A = 거치아이템, 재료 B = 플레이어아이템
    if (Recipe.MaterialA == InTagA && Recipe.MaterialB == InTagB) {
      OutRecipe = &Recipe;
      return true;
    }

    // 2. 순서 무관 매칭
    if (Recipe.MaterialA == InTagB && Recipe.MaterialB == InTagA) {
      OutRecipe = &Recipe;
      return true;
    }
  }
  return false;
}
