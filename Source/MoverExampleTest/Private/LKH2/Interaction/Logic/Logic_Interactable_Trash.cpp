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

	// Intent 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	if (!InteractorProp)
	{
		UE_LOG(LogTemp, Log, TEXT("[TrashLogic] PreCheck 실패: InteractorProp 없음"));
		return false;
	}

	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProp->GetCarriedActor());

	// 1. 유저가 무언가를 들고 있는지 확인
	if (!CarriedItem)
	{
		UE_LOG(LogTemp, Log, TEXT("[TrashLogic] PreCheck 실패: 들고 있는 아이템이 없음"));
		return false;
	}

	// 2. 태그 검사 (버릴 수 없는 물건인지)
	if (UItemData* ItemData = CarriedItem->GetItemData())
	{
		// 컨테이너인 경우 자신은 파괴되지 않고 내용물만 버려지므로, 
		// 컨테이너 자체의 태그가 RestrictedItemTags에 있더라도 허용합니다.
		bool bIsContainer = ContainerTag.IsValid() && ItemData->ItemTag.MatchesTag(ContainerTag);

		if (!bIsContainer && RestrictedItemTags.HasTag(ItemData->ItemTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[TrashLogic] Cannot trash item with restricted tag: [%s]"), *ItemData->ItemTag.ToString());
			return false;
		}
	}

	return true;
}

bool ULogic_Interactable_Trash::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* Manager = Context.Interactor->GetWorld()->GetSubsystem<UInteractionManager>();
	if (!Manager) return false;

	UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	if (!InteractorProp) return false;

	AItemBase* CarriedItem = Cast<AItemBase>(InteractorProp->GetCarriedActor());
	if (!CarriedItem || !CarriedItem->GetItemData()) return false;

	FGameplayTag ItemTag = CarriedItem->GetItemData()->ItemTag;
	UE_LOG(LogTemp, Log, TEXT("[TrashLogic] PerformInteraction: CarriedItem=%s, Tag=%s"), 
		*CarriedItem->GetName(), *ItemTag.ToString());

	// 1. Container 태그 확인
	bool bIsContainer = ContainerTag.IsValid() && ItemTag.MatchesTag(ContainerTag);

	if (bIsContainer)
	{
		UE_LOG(LogTemp, Log, TEXT("[TrashLogic] Container 판정 성공: 내부 아이템 폐기 시도"));
		// 2-A. Container인 경우 내부 아이템들만 폐기
		if (UInteractablePropertyComponent* ContainerProp = CarriedItem->FindComponentByClass<UInteractablePropertyComponent>())
		{
			TArray<FGameplayTag> SlotKeys;
			ContainerProp->StoredItems.GetKeys(SlotKeys);

			int32 TrashCount = 0;
			for (const FGameplayTag& SlotTag : SlotKeys)
			{
				if (AItemBase* SubItem = ContainerProp->GetStoredItem(SlotTag))
				{
					// 특정 슬롯에서 아이템 해제 및 파괴
					ContainerProp->RetrieveItem(SlotTag);
					ContainerProp->DetachTargetItem(SubItem);
					Manager->SafeDestroyItem(SubItem);
					TrashCount++;
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("[TrashLogic] Container(%s)를 비웠습니다. 폐기된 아이템 수: %d"), 
				*CarriedItem->GetName(), TrashCount);
			return true;
		}
	}
	else
	{
		// 2-B. 일반 아이템인 경우 기존처럼 전체 폐기
		Manager->ExecuteTrash(InteractorProp, CarriedItem);
		
		if (CarriedItem)
		{
			UE_LOG(LogTemp, Log, TEXT("[TrashLogic] Trashed item: [%s]"), *CarriedItem->GetName());
		}
		return true;
	}

	return false;
}
