// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Recipe_Combine.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/GameInstance.h"

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

	// 3. 두 아이템의 조합 레시피가 있는지 레시피 매니저에 쿼리
	FGameplayTag MaterialA = CarriedItem->GetItemData()->ItemTag;
	FGameplayTag MaterialB = StoredItem->GetItemData()->ItemTag;

	TArray<FGameplayTag> Inputs {MaterialA, MaterialB};

	UGameInstance* GameInstance = Context.Interactor->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PreInteractCheck Failed: GameInstance is null."));
		return false;
	}

	UItemRecipeManager* RecipeManager = GameInstance->GetSubsystem<UItemRecipeManager>();
	if (!RecipeManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PreInteractCheck Failed: UItemRecipeManager is not valid."));
		return false;
	}

	TArray<FGameplayTag> ResultItemTags = RecipeManager->GetRecipeResultMulti(Inputs, RecipeInteractionTag);

	if (ResultItemTags.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Recipe_Combine] PreInteractCheck Failed: No recipe found for materials [%s] and [%s]."), *MaterialA.ToString(), *MaterialB.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] PreInteractCheck Succeess: Match found for materials [%s] and [%s] -> First Result: [%s]."), *MaterialA.ToString(), *MaterialB.ToString(), *ResultItemTags[0].ToString());
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
	TArray<FGameplayTag> Inputs {MaterialA, MaterialB};

	UItemRecipeManager* RecipeManager = GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>();
	if (!RecipeManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PerformInteraction Failed: RecipeManager not valid."));
		return false;
	}

	TArray<FGameplayTag> ResultItemTags = RecipeManager->GetRecipeResultMulti(Inputs, RecipeInteractionTag);

	if (!ResultItemTags.IsEmpty())
	{
		FGameplayTag PrimaryResult = ResultItemTags[0];
		UE_LOG(LogTemp, Log, TEXT("[Recipe_Combine] PerformInteraction: Combining [%s] and [%s] into [%s]."), *MaterialA.ToString(), *MaterialB.ToString(), *PrimaryResult.ToString());
		InteractionManager->ExecuteCombine(InteractorProperty, TargetProperty, CarriedItem, StoredItem, PrimaryResult, SlotTag);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Recipe_Combine] PerformInteraction: Combine failed. ResultItemTags is Empty."));
	}
	
	return true;
}
