#include "LKH2/Interaction/Logic/Logic_Interactable_Throw.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

bool ULogic_Interactable_Throw::PreInteractCheck(const FInteractionContext& Context)
{
	// 에디터에서 설정한 의도 태그가 일치하는지 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	// 던지기 전에 플레이어가 이 아이템을 현재 들고 있는지 확인
	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	if (!InteractorProperty) return false;

	if (InteractorProperty->GetCarriedActor() != GetOwner())
	{
		return false; // 안 들고 있으면 못 던짐
	}

	return true;
}

bool ULogic_Interactable_Throw::PerformInteraction(const FInteractionContext& Context)
{
	if (UWorld* World = GetWorld())
	{
		if (UInteractionManager* InteractionManager = World->GetSubsystem<UInteractionManager>())
		{
			UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
			AItemBase* CarriedItem = Cast<AItemBase>(GetOwner());
			
			if (InteractorProperty && CarriedItem)
			{
				// 인터랙터 주체의 전방 벡터 방향으로 임펄스 계산
				FVector ThrowDir = FVector::ZeroVector;
				if (AActor* InteractorOwner = InteractorProperty->GetOwner())
				{
					// 카메라이면 더 좋겠지만 일단 소유자 기준 전방
					ThrowDir = InteractorOwner->GetActorForwardVector();
					// 약간 위쪽으로 던지도록 상향력 추가
					ThrowDir += FVector(0.0f, 0.0f, 0.5f);
					ThrowDir.Normalize();
				}
				
				FVector ComputedImpulse = ThrowDir * ThrowForce;
				
				// ExecuteDrop을 통해 물리적 해제 + State 변경 후 Impulse 적용
				InteractionManager->ExecuteDrop(InteractorProperty, CarriedItem, ComputedImpulse);
				return true;
			}
		}
	}
	return false;
}