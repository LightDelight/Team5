// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Carry/Logic/Interface/CarryLogicInterface.h"
#include "LKH2/Recipe/RecipeLogicModuleBase.h"
#include "Logic_CarryInteract_Combine.generated.h"

class UItemData;
class AItemBase;
class UCombineRecipeBook;
struct FCombineRecipe;
class AActor;
class UCarryComponent;

/**
 * 워크스테이션에 아이템을 올리고(거치),
 * 들고 있는 아이템과 거치된 아이템이 레시피와 일치하면 조합(Combine)하는 모듈
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_CarryInteract_Combine
    : public URecipeLogicModuleBase,
      public ICarryLogicInterface {
  GENERATED_BODY()

public:
  ULogic_CarryInteract_Combine();

  virtual void CacheRecipes() override;

  virtual bool OnModuleInteract_Implementation(
      AActor *Interactor, AActor *TargetActor,
      ECarryInteractionType InteractionType) override;

protected:
  /** 이 로직이 블랙보드에 아이템을 거치할 때 사용할 GameplayTag 키입니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine|Blackboard")
  FGameplayTag StoredItemKey;

  /** 허용되는 조합 레시피 목록 책 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine|Recipes")
  TArray<TObjectPtr<UCombineRecipeBook>> RecipeBooks;

  /** 조합 결과로 스폰할 기본 아이템 블루프린트 클래스 (ex: BP_ItemBase) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combine|Spawn")
  TSubclassOf<AItemBase> BaseItemClassToSpawn;

private:
  // 최적화된 빠른 식별을 위해 구조체를 복사해 둔 캐시 배열입니다.
  // 에디터에서 원본 DataAsset의 베열 구조가 변하더라도 크래시가 나지 않게
  // 값으로 저장합니다.
  TArray<FCombineRecipe> CachedRecipes;

  // 보유한 캐시 레시피 중 조건에 부합하는 것이 있는지 검사하고 결과를
  // 반환합니다.
  bool TryFindRecipe(UItemData *InItemA, UItemData *InItemB,
                     const FCombineRecipe *&OutRecipe) const;
};
