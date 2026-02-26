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

	// 플레이어가 현재 객체를 들고 있지 않거나 다른 객체를 들고있으면 손을 먼저 비운다거나 하는
	// 정책은 Manager의 ExecuteEquip 내부에서 처리 가능. (설계 상)
	
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