// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interaction/Data/ItemRecipeData.h"
#include "Engine/DataTable.h"
#include "Algo/Reverse.h"
#include "Algo/Sort.h"

void UItemRecipeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CurrentRecipeDataAsset = nullptr;
	BuildRecipeCache();
}

void UItemRecipeManager::Deinitialize()
{
	CachedRecipes.Empty();
	CurrentRecipeDataAsset = nullptr;
	Super::Deinitialize();
}

void UItemRecipeManager::SetRecipeDataAsset(UItemRecipeDataAsset* NewDataAsset)
{
	CurrentRecipeDataAsset = NewDataAsset;
	BuildRecipeCache();
}

void UItemRecipeManager::BuildRecipeCache()
{
	CachedRecipes.Empty();

	if (!CurrentRecipeDataAsset || !CurrentRecipeDataAsset->RecipeTable)
	{
		return;
	}

	TArray<FRecipeRow*> AllRecipes;
	CurrentRecipeDataAsset->RecipeTable->GetAllRows<FRecipeRow>(TEXT("ItemRecipeManager_BuildCache"), AllRecipes);

	for (const FRecipeRow* Row : AllRecipes)
	{
		if (Row)
		{
			CachedRecipes.Add(*Row);
		}
	}

	// Algo::Reverse를 통한 LIFO 정렬 보정 (동등 우선순위에서 나중에 선언된 것이 먼저 오도록)
	if (CurrentRecipeDataAsset->bSortLIFO)
	{
		Algo::Reverse(CachedRecipes);
	}

	// Priority 기반 정렬 (StableSort를 사용하여 LIFO 순서 보존)
	bool bDesc = CurrentRecipeDataAsset->bSortDescending;
	Algo::StableSort(CachedRecipes, [bDesc](const FRecipeRow& A, const FRecipeRow& B)
	{
		return bDesc ? (B.Priority < A.Priority) : (A.Priority < B.Priority);
	});
}

TArray<FGameplayTag> UItemRecipeManager::GetRecipeResultSingle(FGameplayTag InputItemTag, FGameplayTag InteractionTag) const
{
	TArray<FGameplayTag> SingleInput;
	if (InputItemTag.IsValid())
	{
		SingleInput.Add(InputItemTag);
	}
	return GetRecipeResultMulti(SingleInput, InteractionTag);
}

TArray<FGameplayTag> UItemRecipeManager::GetRecipeResultMulti(const TArray<FGameplayTag>& InputItemTags, FGameplayTag InteractionTag) const
{
	if (CachedRecipes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UItemRecipeManager] 레시피 캐시가 비어 있습니다."));
		return TArray<FGameplayTag>();
	}

	if (InputItemTags.IsEmpty() || !InteractionTag.IsValid())
	{
		return TArray<FGameplayTag>();
	}

	// 이미 정렬/캐싱된 레시피 목록을 순회합니다.
	for (const FRecipeRow& Row : CachedRecipes)
	{
		if (Row.InteractionTag != InteractionTag)
			continue;

		// 개수 비교
		if (Row.InputItemTags.Num() != InputItemTags.Num())
			continue;

		// 태그 구성 비교 (순서 무관 지원을 위해 복사 후 제거 방식 사용)
		TArray<FGameplayTag> TempRowTags = Row.InputItemTags;
		bool bMatch = true;

		for (const FGameplayTag& InTag : InputItemTags)
		{
			int32 FoundIndex = TempRowTags.Find(InTag);
			if (FoundIndex != INDEX_NONE)
			{
				TempRowTags.RemoveAtSwap(FoundIndex);
			}
			else
			{
				// 하나라도 매칭되지 않으면 실패
				bMatch = false;
				break;
			}
		}

		if (bMatch && TempRowTags.IsEmpty())
		{
			return Row.OutputItemTags;
		}
	}

	// 매칭 결과가 없으면 빈 배열 반환
	return TArray<FGameplayTag>();
}
