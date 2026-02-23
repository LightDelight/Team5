// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_CarryInteract_Combine.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
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

void ULogic_CarryInteract_Combine::CacheRecipes() {
  Super::CacheRecipes();

  CachedRecipes.Empty();

  // 1. 책에 있는 레시피 구조체 포인터들을 모두 복사하여 캐싱합니다.
  for (UCombineRecipeBook *Book : RecipeBooks) {
    if (Book) {
      for (const FCombineRecipe &Recipe : Book->Recipes) {
        CachedRecipes.Add(Recipe);
      }
    }
  }

  // 2. 부모 클래스 템플릿 정렬 함수를 활용하여 Priority 기준으로 정리합니다.
  SortRecipes(CachedRecipes,
              [](const FCombineRecipe &A, const FCombineRecipe &B) {
                // 오름차순 기준으로 적습니다. bSortDescending 여부에 따라
                // 내부에서 자동으로 반전되어 최고 Priority가 맨 앞에 오게
                // 됩니다.
                return A.Priority < B.Priority;
              });
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

  UCarryInteractComponent *SnapComp = nullptr;
  if (ILogicContextInterface *Context =
          Cast<ILogicContextInterface>(TargetActor)) {
    SnapComp = Context->GetCarryInteractComponent();
  } else {
    SnapComp = TargetActor->FindComponentByClass<UCarryInteractComponent>();
  }

  if (StoredActor == nullptr) {
    // 워크스테이션이 비어있음 -> 거치 수행
    if (PlayerActor != nullptr) {
      // ContainerItemBase(접시 등)는 수납 대상에서 제외
      if (Cast<AContainerItemBase>(PlayerActor)) {
        return false;
      }
      if (TargetActor->HasAuthority()) {
        CarrierComp->ForceDrop();

        if (UPrimitiveComponent *RootPrim =
                Cast<UPrimitiveComponent>(PlayerActor->GetRootComponent())) {
          RootPrim->SetSimulatePhysics(false);
          RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
        }

        if (UItemStateComponent *StateComp =
                PlayerActor->FindComponentByClass<UItemStateComponent>()) {
          StateComp->SetItemState(EItemState::Stored);
        }

        if (SnapComp) {
          PlayerActor->AttachToComponent(
              SnapComp,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        } else {
          PlayerActor->AttachToActor(
              TargetActor,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }

        Blackboard->ObjectBlackboard.SetObject(StoredItemKey, PlayerActor);
      }
      return true;
    }
  } else {
    // 워크스테이션에 아이템이 거치되어 있음
    if (PlayerActor == nullptr) {
      // 플레이어 손이 비어있음 -> 회수 수행
      if (TargetActor->HasAuthority()) {
        StoredActor->DetachFromActor(
            FDetachmentTransformRules::KeepWorldTransform);
        CarrierComp->ForceEquip(StoredActor);

        if (UItemStateComponent *StateComp =
                StoredActor->FindComponentByClass<UItemStateComponent>()) {
          StateComp->SetItemState(EItemState::Carried);
        }

        Blackboard->ObjectBlackboard.SetObject(StoredItemKey, nullptr);
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
          // 레시치 매칭 성공
          if (TargetActor->HasAuthority()) {
            CarrierComp->ForceDrop(); // 손에서 내려놓기 처리

            UWorld *World = TargetActor->GetWorld();
            UItemManagerSubsystem *ItemMgr =
                World ? World->GetSubsystem<UItemManagerSubsystem>()
                      : nullptr;

            if (World && ItemMgr) {
              FTransform SpawnTransform =
                  SnapComp ? SnapComp->GetComponentTransform()
                           : TargetActor->GetActorTransform();

              // 기존 아이템 대상 파괴 (ItemManager 추적)
              ItemMgr->DestroyItem(PlayerItem);
              ItemMgr->DestroyItem(StoredItem);

              // 결과 아이템 스폰 (ItemManager 파이프라인)
              AItemBase *NewItem = ItemMgr->SpawnItemFromData(
                  FoundRecipe->ResultItemData, SpawnTransform,
                  BaseItemClassToSpawn);

              if (NewItem) {
                // 서버 사이드 물리 끄기 및 콜리전 해제 (초기화)
                if (UPrimitiveComponent *RootPrim = Cast<UPrimitiveComponent>(
                        NewItem->GetRootComponent())) {
                  RootPrim->SetSimulatePhysics(false);
                  RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
                }

                if (UItemStateComponent *StateComp =
                        NewItem->FindComponentByClass<UItemStateComponent>()) {
                  StateComp->SetItemState(EItemState::Stored);
                }

                // 스폰이 완료된 이후에 부착을 진행해야 부착 상태가 유실되지
                // 않음
                if (SnapComp) {
                  NewItem->AttachToComponent(
                      SnapComp,
                      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                } else {
                  NewItem->AttachToActor(
                      TargetActor,
                      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                }

                // 블랙보드에 신규 거치 아이템 등록
                Blackboard->ObjectBlackboard.SetObject(StoredItemKey, NewItem);
              }
            }
            return true;
          }
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

    // 2. 순서 무관 매칭: 재료 B = 거치아이템, 재료 A = 플레이어아이템
    // 구조체 확장 전에는 bOrderIndependent 가 없으므로 생략 혹은 확장
    if (Recipe.MaterialA == InItemB && Recipe.MaterialB == InItemA) {
      OutRecipe = &Recipe;
      return true;
    }
  }
  return false;
}
