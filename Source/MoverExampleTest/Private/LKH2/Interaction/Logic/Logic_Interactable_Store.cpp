// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Store.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"

ULogic_Interactable_Store::ULogic_Interactable_Store()
{
}

bool ULogic_Interactable_Store::PreInteractCheck(const FInteractionContext &Context)
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

	// 플레이어가 아이템을 들고 있지 않으면 실패
	AActor* CarriedActor = InteractorProperty->GetCarriedActor();
	AItemBase* CarriedItem = Cast<AItemBase>(CarriedActor);
	if (!CarriedItem)
	{
		return false;
	}

	// 해당 슬롯에 이미 아이템이 존재하면 실패
	if (TargetProperty->HasItem(SlotTag))
	{
		return false;
	}

	// 들고 있는 아이템이 수납 불가능한 태그를 가지고 있으면 실패
	if (!RestrictedItemTags.IsEmpty() && CarriedItem->GetItemData())
	{
		if (RestrictedItemTags.HasTagExact(CarriedItem->GetItemData()->ItemTag))
		{
			return false;
		}
	}

	return true;
}

bool ULogic_Interactable_Store::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
	if (!InteractionManager)
	{
		return false;
	}

	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProperty->GetCarriedActor());

	InteractionManager->SafeStoreHandItem(InteractorProperty, TargetProperty, CarriedItem, SlotTag);

	return true;
}
