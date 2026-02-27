#include "LKH2/Interaction/Logic/Logic_Interactable_Drop.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "Engine/World.h"

bool ULogic_Interactable_Drop::PreInteractCheck(const FInteractionContext &Context)
{
	// 에디터에서 설정한 의도 태그가 일치하는지 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	// 플레이어가 현재 들고 있는 객체여야 함
	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	if (!InteractorProperty) return false;

	AActor* CurrentOwner = GetOwner();
	if (!CurrentOwner || InteractorProperty->GetCarriedActor() != CurrentOwner)
	{
		return false;
	}

	return true;
}

bool ULogic_Interactable_Drop::PerformInteraction(const FInteractionContext &Context)
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionManager* InteractionManager = World->GetSubsystem<UInteractionManager>())
		{
			UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
			AItemBase* CarriedItem = Cast<AItemBase>(GetOwner());
			
			if (InteractorProperty && CarriedItem)
			{
				// ItemManagerSubsystem의 ThrowTargetItem 로직과 통합하거나, 여기선 단순히 0벡터(그냥 놓기)로 호출
				InteractionManager->ExecuteDrop(InteractorProperty, CarriedItem, FVector::ZeroVector);
				return true;
			}
		}
	}
	return false;
}