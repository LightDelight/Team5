// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Container_AddItem.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"

bool ULogic_Container_AddItem::PreInteractCheck(const FInteractionContext &Context)
{
	// Intent 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	// Container 자신의 PropertyComponent
	UInteractablePropertyComponent* ContainerProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	if (!ContainerProperty)
	{
		return false;
	}

	// 대상 액터 확인 (ProcessInputBuffer에서 BestTarget으로 전달됨)
	AActor* TargetActor = Context.TargetActor;
	if (!TargetActor)
	{
		return false;
	}

	// Container 자신의 PropertyComponent
	AActor* ContainerActor = ContainerProperty ? ContainerProperty->GetOwner() : nullptr;

	// 자기 자신을 담는 것을 방지
	if (TargetActor == ContainerActor)
	{
		// UE_LOG(LogTemp, Log, TEXT("[Container_AddItem] PreCheck 실패: 자기 자신을 수납할 수 없음"));
		return false;
	}
	AItemBase* SelfItem = Cast<AItemBase>(ContainerActor);
	if (!SelfItem)
	{
		return false;
	}

	// 빈 슬롯 확인
	FGameplayTag AvailableSlot = FindAvailableSlot(ContainerProperty);
	if (!AvailableSlot.IsValid())
	{
		return false;
	}

	// 대상에서 담을 아이템 찾기
	AItemBase* ItemToAdd = nullptr;
	
	// 경로 A: 대상이 ItemBase (바닥 아이템 등)
	ItemToAdd = Cast<AItemBase>(TargetActor);

	// 경로 B: 대상이 Workstation — 작업대의 StoredItem 확인
	if (!ItemToAdd && WorkstationSlotTag.IsValid())
	{
		if (TargetActor->Implements<UInteractionContextInterface>())
		{
			if (UInteractablePropertyComponent* TargetProp = TargetActor->FindComponentByClass<UInteractablePropertyComponent>())
			{
				ItemToAdd = TargetProp->GetStoredItem(WorkstationSlotTag);
			}
		}
	}

	if (!ItemToAdd)
	{
		return false;
	}

	// 필터 확인
	if (!CanAcceptItem(ItemToAdd))
	{
		return false;
	}

	return true;
}

bool ULogic_Container_AddItem::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
	if (!InteractionManager) return false;

	UInteractablePropertyComponent* ContainerProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	UInteractorPropertyComponent* InteractorProperty = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
	AActor* TargetActor = Context.TargetActor;
	AActor* ContainerActor = ContainerProperty ? ContainerProperty->GetOwner() : nullptr;

	// 자기 자신을 담는 것을 방지
	if (TargetActor == ContainerActor)
	{
		return false;
	}

	FGameplayTag AvailableSlot = FindAvailableSlot(ContainerProperty);

	AItemBase* ItemToAdd = nullptr;

	// 경로 A: 대상이 ItemBase
	ItemToAdd = Cast<AItemBase>(TargetActor);
	if (ItemToAdd)
	{
		// 바닥 아이템 → Container에 담기 (World -> Container)
		InteractionManager->SafeStoreWorldItem(ContainerProperty, ItemToAdd, AvailableSlot);
		return true;
	}

	// 경로 B: 대상이 Workstation
	if (WorkstationSlotTag.IsValid())
	{
		if (UInteractablePropertyComponent* TargetProp = TargetActor->FindComponentByClass<UInteractablePropertyComponent>())
		{
			// 작업대 -> Container (Transfer 사용)
			if (InteractionManager->SafeStoreTransferItem(TargetProp, WorkstationSlotTag, ContainerProperty, AvailableSlot))
			{
				return true;
			}
		}
	}

	return false;
}

bool ULogic_Container_AddItem::CanAcceptItem(AItemBase* Item) const
{
	if (!Item || !Item->GetItemData()) return false;

	FGameplayTag ItemTag = Item->GetItemData()->ItemTag;

	// 거부 필터
	if (!RejectedItemTags.IsEmpty() && RejectedItemTags.HasTagExact(ItemTag))
	{
		return false;
	}

	// 허용 필터 (비어있으면 모두 허용)
	if (!AcceptedItemTags.IsEmpty() && !AcceptedItemTags.HasTagExact(ItemTag))
	{
		return false;
	}

	return true;
}

FGameplayTag ULogic_Container_AddItem::FindAvailableSlot(UInteractablePropertyComponent* ContainerProperty) const
{
	if (!ContainerProperty) return FGameplayTag();

	for (const FGameplayTag& SlotTag : SlotTags)
	{
		if (SlotTag.IsValid() && !ContainerProperty->HasItem(SlotTag))
		{
			return SlotTag;
		}
	}

	return FGameplayTag(); // 빈 슬롯 없음
}
