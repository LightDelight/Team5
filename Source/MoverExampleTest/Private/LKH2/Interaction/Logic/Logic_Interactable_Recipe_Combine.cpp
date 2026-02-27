// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Recipe_Combine.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"

void ULogic_Interactable_Recipe_Combine::CacheRecipes(const ULogicEntityDataBase* EntityData)
{
	CachedRecipes.Empty();

	// 넘겨받은 EntityData가 없다면 (PostLoad나 Editor 환경), 자신의 Outer에서 시도
	const ULogicEntityDataBase* TargetEntityData = EntityData;
	if (!TargetEntityData)
	{
		TargetEntityData = Cast<ULogicEntityDataBase>(GetOuter());
	}
	
	if (!TargetEntityData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] CacheRecipes Failed: TargetEntityData is null. Outer is [%s]"), GetOuter() ? *GetOuter()->GetName() : TEXT("NULL"));
		return;
	}

	const FItemStatValue* StatValue = TargetEntityData->EntityStats.Find(RecipeTag);
	if (!StatValue)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] CacheRecipes Failed: Could not find RecipeTag [%s] in EntityStats."), *RecipeTag.ToString());
		return;
	}

	// 타입이 Object Array인지 확인
	if (StatValue->Type != EItemStatType::ObjectArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] CacheRecipes Failed: StatType for RecipeTag [%s] is not ObjectArray."), *RecipeTag.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] CacheRecipes: Found ObjectArray with [%d] elements for tag [%s]."), StatValue->ObjectArrayValue.Num(), *RecipeTag.ToString());

	for (UObject* Obj : StatValue->ObjectArrayValue)
	{
		// CombineRecipeBook으로 캐스팅하여 레시피 추출
		if (UCombineRecipeBook* RecipeBook = Cast<UCombineRecipeBook>(Obj))
		{
			CachedRecipes.Append(RecipeBook->Recipes);
			UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] CacheRecipes: Appended [%d] recipes from RecipeBook [%s]. Total Cached: [%d]"), RecipeBook->Recipes.Num(), *RecipeBook->GetName(), CachedRecipes.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] CacheRecipes: Object [%s] is not a UCombineRecipeBook."), Obj ? *Obj->GetName() : TEXT("NULL"));
		}
	}

	// 부모의 범용 템플릿 정렬 함수 호출 (Priority 기준)
	// Predicate는 오름차순(A < B)으로 작성. bSortDescending 값에 따라 부모가 변경하여 정렬함.
	SortRecipes(CachedRecipes, [](const FCombineRecipe& A, const FCombineRecipe& B) {
		return A.Priority < B.Priority;
	});
	
	UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] CacheRecipes: Successfully cached and sorted [%d] recipes."), CachedRecipes.Num());
}

bool ULogic_Interactable_Recipe_Combine::PreInteractCheck(const FInteractionContext &Context)
{
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: RequiredIntentTag mismatch."));
		return false;
	}

	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);

	if (!InteractorProperty || !TargetProperty)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: PropertyComponents not found."));
		return false;
	}

	// 1. 플레이어가 들고 있는 아이템 확인
	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProperty->GetCarriedActor());
	if (!CarriedItem || !CarriedItem->GetItemData())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: Player is not holding any valid item."));
		return false;
	}

	// 2. 대상 액터(워크스테이션 등)가 SlotTag에 보관 중인 아이템 확인
	AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag);
	if (!StoredItem || !StoredItem->GetItemData())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: No valid item stored in slot [%s]."), *SlotTag.ToString());
		return false;
	}

	// 3. 두 아이템의 조합 레시피가 있는지 확인
	FGameplayTag MaterialA = CarriedItem->GetItemData()->ItemTag;
	FGameplayTag MaterialB = StoredItem->GetItemData()->ItemTag;

	bool bFoundRecipe = false;
	for (const FCombineRecipe& Recipe : CachedRecipes)
	{
		// 순서에 상관없이 재료가 일치하는지 확인
		if ((Recipe.MaterialA.MatchesTagExact(MaterialA) && Recipe.MaterialB.MatchesTagExact(MaterialB)) ||
			(Recipe.MaterialA.MatchesTagExact(MaterialB) && Recipe.MaterialB.MatchesTagExact(MaterialA)))
		{
			bFoundRecipe = true;
			break;
		}
	}

	if (!bFoundRecipe)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: No recipe found for materials [%s] and [%s]."), *MaterialA.ToString(), *MaterialB.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] PreInteractCheck Succeess: Match found for materials [%s] and [%s]."), *MaterialA.ToString(), *MaterialB.ToString());
	return true;
}

bool ULogic_Interactable_Recipe_Combine::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
	if (!InteractionManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PerformInteraction Failed: InteractionManager is null."));
		return false;
	}

	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);

	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProperty->GetCarriedActor());
	AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag);

	FGameplayTag MaterialA = CarriedItem->GetItemData()->ItemTag;
	FGameplayTag MaterialB = StoredItem->GetItemData()->ItemTag;

	FGameplayTag ResultItemTag;

	for (const FCombineRecipe& Recipe : CachedRecipes)
	{
		if ((Recipe.MaterialA.MatchesTagExact(MaterialA) && Recipe.MaterialB.MatchesTagExact(MaterialB)) ||
			(Recipe.MaterialA.MatchesTagExact(MaterialB) && Recipe.MaterialB.MatchesTagExact(MaterialA)))
		{
			ResultItemTag = Recipe.ResultItemTag;
			break;
		}
	}

	if (ResultItemTag.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] PerformInteraction: Combining [%s] and [%s] into [%s]."), *MaterialA.ToString(), *MaterialB.ToString(), *ResultItemTag.ToString());
		InteractionManager->ExecuteCombine(InteractorProperty, TargetProperty, CarriedItem, StoredItem, ResultItemTag, SlotTag);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PerformInteraction: Combine failed. ResultItemTag is Invalid."));
	}
	
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
