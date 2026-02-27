// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Recipe/RecipeLogicModuleBase.h"
#include "LKH2/Interaction/Recipe/CombineRecipeBook.h"
#include "Logic_Interactable_Recipe_Combine.generated.h"

/**
 * 조합(Combine) 상호작용 관련 로직 모듈.
 * 워크스테이션 또는 아이템 DataAsset의 EntityStats에서 RecipeTag를 이용해 Object Array를 받아오고,
 * 그 안의 CombineRecipeBook들을 읽어 캐싱합니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Combine Recipe"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Recipe_Combine : public URecipeLogicModuleBase
{
	GENERATED_BODY()

public:

	// 레시피 북 배열을 가져올 스탯 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag RecipeTag;

	// 조합할 대상 아이템이 보관된 슬롯 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag SlotTag;

protected:
	// 캐싱된 조합 레시피 목록 (우선순위 순으로 정렬됨)
	UPROPERTY(Transient)
	TArray<FCombineRecipe> CachedRecipes;

	virtual void CacheRecipes(const class ULogicEntityDataBase* EntityData = nullptr) override;

public:
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
	
	virtual TArray<FGameplayTag> GetRequiredStatTags() const override;
};
