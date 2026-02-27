#include "LKH2/Interaction/Logic/Logic_Interactable_PickUp.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "Engine/World.h"

bool ULogic_Interactable_PickUp::PreInteractCheck(const FInteractionContext &Context)
{
	// 에디터에서 설정한 의도 태그가 일치하는지 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	// 아이템을 이미 들고 있는 경우 줍기 거부
	if (UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp))
	{
		if (InteractorProperty->GetCarriedActor() != nullptr)
		{
			return false;
		}
	}
	
	return true;
}

bool ULogic_Interactable_PickUp::PerformInteraction(const FInteractionContext &Context)
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionManager* InteractionManager = World->GetSubsystem<UInteractionManager>())
		{
			UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
			AItemBase* ItemToEquip = Cast<AItemBase>(GetOwner());
			
			if (InteractorProperty && ItemToEquip)
			{
				InteractionManager->ExecuteEquip(InteractorProperty, ItemToEquip);
				return true;
			}
		}
	}
	return false;
}