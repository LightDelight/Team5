// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Recipe_Combine.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"

ULogic_Interactable_Recipe_Combine::ULogic_Interactable_Recipe_Combine()
{
}

void ULogic_Interactable_Recipe_Combine::CacheRecipes()
{
	CachedRecipes.Empty();

	// 자신의 Outer(DataAsset 등)에서 EntityStats 가져오기
	if (ULogicEntityDataBase* EntityData = Cast<ULogicEntityDataBase>(GetOuter()))
	{
		if (FItemStatValue* StatValue = EntityData->EntityStats.Find(RecipeTag))
		{
			// 타입이 Object Array인지 확인
			if (StatValue->Type == EItemStatType::ObjectArray)
			{
				for (UObject* Obj : StatValue->ObjectArrayValue)
				{
					// CombineRecipeBook으로 캐스팅하여 레시피 추출
					if (UCombineRecipeBook* RecipeBook = Cast<UCombineRecipeBook>(Obj))
					{
						CachedRecipes.Append(RecipeBook->Recipes);
					}
				}
			}
		}
	}

	// 부모의 범용 템플릿 정렬 함수 호출 (Priority 기준)
	// Predicate는 오름차순(A < B)으로 작성. bSortDescending 값에 따라 부모가 변경하여 정렬함.
	SortRecipes(CachedRecipes, [](const FCombineRecipe& A, const FCombineRecipe& B) {
		return A.Priority < B.Priority;
	});
}

bool ULogic_Interactable_Recipe_Combine::PreInteractCheck(const FInteractionContext &Context)
{
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	return true;
}

bool ULogic_Interactable_Recipe_Combine::PerformInteraction(const FInteractionContext &Context)
{
	// 실제 Combine 기능에 관련된 핵심 동작 요청 등의 코드를 추후 작성
	
	return true;
}

TArray<FGameplayTag> ULogic_Interactable_Recipe_Combine::GetRequiredStatTags() const
{
	TArray<FGameplayTag> Tags;
	if (RecipeTag.IsValid())
	{
		Tags.Add(RecipeTag);
	}
	return Tags;
}
