// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Trash.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"

bool ULogic_Interactable_Trash::PreInteractCheck(const FInteractionContext &Context)
{
	if (!Super::PreInteractCheck(Context))
	{
		return false;
	}

	UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	if (!InteractorProp)
	{
		return false;
	}

	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProp->GetCarriedActor());

	// 1. 유저가 쓰레기를 들고 있는지 등 폐기가 가능한 조건 검사
	if (!CarriedItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TrashLogic] Iteractor is not holding an item."));
		return false;
	}

	// 2. 태그 검사 (버릴 수 없는 물건인지)
	if (UItemData* ItemData = CarriedItem->GetItemData())
	{
		if (RestrictedItemTags.HasTag(ItemData->ItemTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[TrashLogic] Cannot trash item with restricted tag: [%s]"), *ItemData->ItemTag.ToString());
			return false;
		}
	}

	return true;
}

bool ULogic_Interactable_Trash::PerformInteraction(const FInteractionContext &Context)
{
	if (UInteractionManager* Manager = Context.Interactor->GetWorld()->GetSubsystem<UInteractionManager>())
	{
		UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
		if (InteractorProp)
		{
			AItemBase* CarriedItem = Cast<AItemBase>(InteractorProp->GetCarriedActor());
			Manager->ExecuteTrash(InteractorProp, CarriedItem);
			
			if (CarriedItem)
			{
				UE_LOG(LogTemp, Log, TEXT("[TrashLogic] Trashed item: [%s]"), *CarriedItem->GetName());
			}
			return true;
		}
	}

	return false;
}
