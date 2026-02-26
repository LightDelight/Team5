// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"

// Sets default values for this component's properties
UInteractablePropertyComponent::UInteractablePropertyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	// bWantsInitializeComponent = true; // 필요 시
}

// Called when the game starts
void UInteractablePropertyComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void UInteractablePropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInteractablePropertyComponent::TryStoreItem(FGameplayTag SlotTag, AItemBase* ItemToStore)
{
	if (!SlotTag.IsValid() || !ItemToStore)
	{
		return false;
	}

	// 이미 해당 슬롯에 누군가 거치되어 있다면 실패
	if (StoredItems.Contains(SlotTag) && StoredItems[SlotTag] != nullptr)
	{
		return false;
	}

	StoredItems.Add(SlotTag, ItemToStore);
	return true;
}

AItemBase* UInteractablePropertyComponent::RetrieveItem(FGameplayTag SlotTag)
{
	if (!SlotTag.IsValid())
	{
		return nullptr;
	}

	if (StoredItems.Contains(SlotTag) && StoredItems[SlotTag] != nullptr)
	{
		AItemBase* Item = StoredItems[SlotTag];
		StoredItems[SlotTag] = nullptr;
		return Item;
	}

	return nullptr;
}

AItemBase* UInteractablePropertyComponent::GetStoredItem(FGameplayTag SlotTag) const
{
	if (StoredItems.Contains(SlotTag))
	{
		return StoredItems[SlotTag];
	}
	return nullptr;
}

bool UInteractablePropertyComponent::HasItem(FGameplayTag SlotTag) const
{
	if (StoredItems.Contains(SlotTag))
	{
		return StoredItems[SlotTag] != nullptr;
	}
	return false;
}

void UInteractablePropertyComponent::AttachTargetItem(AItemBase* ItemToStore)
{
	if (!ItemToStore)
	{
		return;
	}

	if (AActor* TargetActor = GetOwner())
	{
		ItemToStore->AttachToActor(
			TargetActor,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void UInteractablePropertyComponent::DetachTargetItem(AItemBase* ItemToStore)
{
	if (!ItemToStore)
	{
		return;
	}

	ItemToStore->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}
