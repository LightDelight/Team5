// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"

#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

void UInteractionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 초기화 로직 필요 시 추가
}

void UInteractionManager::Deinitialize()
{
	// 정리 로직 필요 시 추가
	Super::Deinitialize();
}

void UInteractionManager::ExecuteEquip(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToEquip)
{
	if (!InteractorProperty || !ItemToEquip) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. ItemManagerSubsystem을 통해 논리적 상태 갱신 (PickUpItem) - 물리를 먼저 끔
	ItemManager->PickUpItem(ItemToEquip->GetInstanceId());

	// 2. InteractorPropertyComponent를 통해 물리적 부착 (ForceEquip)
	InteractorProperty->ForceEquip(ItemToEquip);
}

void UInteractionManager::ExecuteDrop(UInteractorPropertyComponent* InteractorProperty, AItemBase* CarriedItem, FVector OptionalImpulse)
{
	if (!InteractorProperty || !CarriedItem) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. InteractorPropertyComponent를 통해 물리적 해제 및 위치 보정 (ForceDrop)
	InteractorProperty->ForceDrop();

	// 2. ItemManagerSubsystem을 통해 논리적 상태 갱신 (DropItem)
	ItemManager->DropItem(CarriedItem->GetInstanceId());

	// 3. OptionalImpulse가 있다면 ItemManagerSubsystem의 ThrowTargetItem 호출
	if (!OptionalImpulse.IsZero())
	{
		ItemManager->ThrowTargetItem(CarriedItem->GetInstanceId(), OptionalImpulse);
	}
}

void UInteractionManager::ExecuteStore(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag)
{
	if (!InteractorProperty || !TargetProperty || !ItemToStore) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. Interactor에서 아이템 분리 (물리적 해제)
	InteractorProperty->ForceDrop();

	// 2. InteractablePropertyComponent에 시각적/물리적 부착 (AttachTargetItem)
	if (TargetProperty->TryStoreItem(SlotTag, ItemToStore))
	{
		TargetProperty->AttachTargetItem(ItemToStore);
		// 3. ItemManagerSubsystem을 통해 논리적 상태 갱신 (StoreItem)
		ItemManager->StoreItem(ItemToStore->GetInstanceId());
	}
}

void UInteractionManager::ExecuteRetrieve(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToRetrieve, FGameplayTag SlotTag)
{
	if (!InteractorProperty || !TargetProperty || !ItemToRetrieve) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. InteractablePropertyComponent에서 시각적/물리적 해제 및 아이템 꺼내기
	AItemBase* RetrievedItem = TargetProperty->RetrieveItem(SlotTag);
	if (RetrievedItem == ItemToRetrieve)
	{
		TargetProperty->DetachTargetItem(ItemToRetrieve);

		// 2. Interactor에 물리적 부착 (PickUp)
		InteractorProperty->ForceEquip(ItemToRetrieve);
		
		// 3. ItemManagerSubsystem을 통해 논리적 상태 갱신 (RetrieveItem)
		ItemManager->RetrieveItem(ItemToRetrieve->GetInstanceId());
	}
}

void UInteractionManager::ExecuteCombine(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* MaterialA, AItemBase* MaterialB, FGameplayTag ResultItemTag, FGameplayTag TargetSlotTag)
{
	if (!InteractorProperty || !TargetProperty || !MaterialA || !MaterialB) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. 재료 아이템들을 물리적/시각적으로 분리
	InteractorProperty->ForceDrop();
	TargetProperty->RetrieveItem(TargetSlotTag); // 슬롯을 비움 (MaterialB)
	TargetProperty->DetachTargetItem(MaterialB);

	// 2. 재료 아이템 상태 파괴 및 소멸 요청
	ItemManager->DestroyItem(MaterialA->GetInstanceId());
	ItemManager->DestroyItem(MaterialB->GetInstanceId());

	// 3. 결과 아이템 생성 요청
	AActor* TargetActor = TargetProperty->GetOwner();
	FTransform SpawnTransform = TargetActor ? TargetActor->GetActorTransform() : FTransform::Identity;
	FGuid NewItemId = ItemManager->SpawnItem(ResultItemTag, SpawnTransform);

	if (NewItemId.IsValid())
	{
		AItemBase* NewItem = ItemManager->GetItemActor(NewItemId);
		if (NewItem)
		{
			// 4. 즉시 Result Item 보관 (StoreItem 로직 재사용)
			if (TargetProperty->TryStoreItem(TargetSlotTag, NewItem))
			{
				TargetProperty->AttachTargetItem(NewItem);
				ItemManager->StoreItem(NewItemId);
			}
		}
	}
}

void UInteractionManager::ExecuteTrash(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToTrash)
{
	if (!InteractorProperty || !ItemToTrash) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. InteractorPropertyComponent를 통해 물리적으로 분리
	InteractorProperty->ForceDrop();

	// 2. ItemManagerSubsystem을 통해 아이템 상태 파괴 및 소멸 요청
	ItemManager->DestroyItem(ItemToTrash->GetInstanceId());
}

void UInteractionManager::ExecuteVending(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, FGameplayTag ItemToSpawnTag)
{
	if (!InteractorProperty || !TargetProperty) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. 결과 아이템 생성 요청
	AActor* TargetActor = TargetProperty->GetOwner();
	FTransform SpawnTransform = TargetActor ? TargetActor->GetActorTransform() : FTransform::Identity;
	FGuid NewItemId = ItemManager->SpawnItem(ItemToSpawnTag, SpawnTransform);

	if (NewItemId.IsValid())
	{
		AItemBase* NewItem = ItemManager->GetItemActor(NewItemId);
		if (NewItem)
		{
			// 2. 생성된 아이템 즉시 줍기 (물리적 부착, 논리적 상태 갱신)
			ItemManager->PickUpItem(NewItemId);
			InteractorProperty->ForceEquip(NewItem);
		}
	}
}
