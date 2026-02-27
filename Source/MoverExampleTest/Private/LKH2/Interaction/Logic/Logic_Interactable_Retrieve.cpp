// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Retrieve.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"

ULogic_Interactable_Retrieve::ULogic_Interactable_Retrieve()
{
}

bool ULogic_Interactable_Retrieve::PreInteractCheck(const FInteractionContext &Context)
{
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);

	if (!InteractorProperty || !TargetProperty)
	{
		return false;
	}

	// 플레이어가 이미 물건을 들고 있으면 실패
	if (InteractorProperty->GetCarriedActor() != nullptr)
	{
		return false;
	}

	// 대상에 보관된 아이템이 없으면 실패
	if (!TargetProperty->HasItem(SlotTag))
	{
		return false;
	}

	return true;
}

bool ULogic_Interactable_Retrieve::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
	if (!InteractionManager)
	{
		return false;
	}

	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);

	AItemBase* ItemToRetrieve = TargetProperty->GetStoredItem(SlotTag);
	if (ItemToRetrieve)
	{
		InteractionManager->ExecuteRetrieve(InteractorProperty, TargetProperty, ItemToRetrieve, SlotTag);
	}

	return true;
}
