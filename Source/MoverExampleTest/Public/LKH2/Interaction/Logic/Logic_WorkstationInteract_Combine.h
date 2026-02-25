// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interaction/Base/LogicInteractionInterface.h"
#include "LKH2/Interaction/Recipe/CombineRecipeBook.h"
#include "LKH2/Interaction/Recipe/RecipeLogicModuleBase.h"
#include "Logic_WorkstationInteract_Combine.generated.h"

class UItemData;
class AItemBase;
class AActor;
class UInteractorComponent;

/**
 * 워크스테이션에 아이템을 올리고(거치),
 * 들고 있는 아이템과 거치된 아이템이 레시피와 일치하면 조합(Combine)하는 모듈.
 * 스폰 클래스는 레시피 ResultItemData의 VisualPreset에서 가져옴.
 *
 * 필요 Stats 태그:
 *   - RecipeBooksTag (ObjectArray → UCombineRecipeBook* 배열)
 *   - StoredItemKey (블랙보드 키)
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_WorkstationInteract_Combine
    : public URecipeLogicModuleBase {
  GENERATED_BODY()

public:
  ULogic_WorkstationInteract_Combine();

  virtual void CacheRecipes() override;

  /** BeginPlay 시 Stats에서 레시피 책을 캐싱 */
  virtual void InitializeLogic(AActor *InOwnerActor) override;

  /** 필수 Stats 태그 선언 */
  virtual TArray<FGameplayTag> GetRequiredStatTags() const override;

protected:
  /** [Core-Logic] 실제 조합 로직 수행 */
  virtual bool PerformInteraction(const FInteractionContext &Context) override;

  // ─── 필요 태그 선언 ───

  /** 이 로직이 블랙보드에 아이템을 거치할 때 사용할 GameplayTag 키.
   *  Stats(WorkstationStats 등)에 동일한 태그를 키로 하고, Tag 타입의 값을 설정하면
   *  해당 값을 실제 블랙보드 키로 우선 사용합니다. (Key Aliasing) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Combine|Blackboard")
  FGameplayTag StoredItemKey;

  /** Stats에서 레시피 책(UCombineRecipeBook*) 배열을 조회할 태그 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Combine|RequiredTags")
  FGameplayTag RecipeBooksTag;

private:
  TArray<FCombineRecipe> CachedRecipes;

  bool TryFindRecipe(FGameplayTag InTagA, FGameplayTag InTagB,
                     const FCombineRecipe *&OutRecipe) const;
};
