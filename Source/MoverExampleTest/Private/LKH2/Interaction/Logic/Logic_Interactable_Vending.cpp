// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Vending.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"

#include "LKH2/Interactables/Item/ItemData.h"

bool ULogic_Interactable_Vending::PreInteractCheck(const FInteractionContext &Context)
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

	// 1. 유저가 빈 손인지 확인
	if (InteractorProp->GetCarriedActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VendingLogic] Iteractor is already holding an item. Cannot vend."));
		return false;
	}

	// 2. 나오는 아이템 태그가 설정되어 있는지 확인
	if (!ItemToVend.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VendingLogic] Vending item tag is not set."));
		return false;
	}

	return true;
}

bool ULogic_Interactable_Vending::PerformInteraction(const FInteractionContext &Context)
{
	if (UInteractionManager* Manager = Context.Interactor->GetWorld()->GetSubsystem<UInteractionManager>())
	{
		UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
		UInteractablePropertyComponent* TargetProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);

		if (InteractorProp && TargetProp && ItemToVend.IsValid())
		{
			Manager->ExecuteVending(InteractorProp, TargetProp, ItemToVend);
			UE_LOG(LogTemp, Log, TEXT("[VendingLogic] Vended item: [%s]"), *ItemToVend.ToString());
			return true;
		}
	}

	return false;
}
