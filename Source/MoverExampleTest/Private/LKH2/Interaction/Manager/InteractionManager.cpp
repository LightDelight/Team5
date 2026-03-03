// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"

#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "Engine/World.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interactables/Item/ItemSmoothingComponent.h"

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

void UInteractionManager::SafePickUpItem(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToEquip)
{
	if (!InteractorProperty || !ItemToEquip) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. 물리 비활성화를 위한 상태 변경 먼저
	ItemManager->PickUpItem(ItemToEquip->GetInstanceId());

	// 2. 물리 비활성화 상태에서 부착
	InteractorProperty->ForceEquip(ItemToEquip);

	// 3. 수동 부착 복제 정보 갱신
	if (UItemStateComponent* StateComp = ItemToEquip->GetStateComponent())
	{
		StateComp->UpdateAttachmentReplication();
	}

	// 4. 스무딩 상태 즉시 갱신 (로컬 제어권에 따른 Snap 보장)
	if (UItemSmoothingComponent* SmoothingComp = ItemToEquip->FindComponentByClass<UItemSmoothingComponent>())
	{
		SmoothingComp->SetSmoothingEnabled(true); // 내부적으로 ApplySmoothingState 호출
	}
}

void UInteractionManager::SafeDropItem(UInteractorPropertyComponent* InteractorProperty, AItemBase* CarriedItem, FVector Impulse)
{
	if (!InteractorProperty || !CarriedItem) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. 물리적 해제
	InteractorProperty->ForceDrop();

	// 2. 논리적 상태 갱신
	ItemManager->DropItem(CarriedItem->GetInstanceId());

	// 3. 추가 임펄스 적용
	if (!Impulse.IsZero())
	{
		ItemManager->ThrowTargetItem(CarriedItem->GetInstanceId(), Impulse);
	}

	// 3. 수동 부착 복제 정보 갱신 (부모 해제)
	if (UItemStateComponent* StateComp = CarriedItem->GetStateComponent())
	{
		StateComp->UpdateAttachmentReplication();
	}
}

bool UInteractionManager::SafeStoreHandItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag)
{
	if (!InteractorProperty || !TargetProperty || !ItemToStore) return false;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return false;

	// Interactor에서 먼저 분리 (물리적 해제)
	InteractorProperty->ForceDrop();

	// 1. 슬롯 보관 시도
	if (TargetProperty->TryStoreItem(SlotTag, ItemToStore))
	{
		// 1. 상태 변경 먼저 (이때 SimulatePhysics가 false로 설정됨)
		ItemManager->StoreItem(ItemToStore->GetInstanceId());

		// 2. 물리 비활성화 확인 후 부착
		UInteractableComponent* SnapTarget = TargetProperty->GetOwner()
			? TargetProperty->GetOwner()->FindComponentByClass<UInteractableComponent>()
			: nullptr;
		TargetProperty->AttachTargetItem(ItemToStore, SnapTarget);

		// 3. 수동 부착 복제 정보 갱신
		if (UItemStateComponent* StateComp = ItemToStore->GetStateComponent())
		{
			StateComp->UpdateAttachmentReplication();
		}

		return true;
	}

	return false;
}

bool UInteractionManager::SafeStoreWorldItem(UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag)
{
	if (!TargetProperty || !ItemToStore) return false;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return false;

	// Interactor 관여 없음 (바닥 아이템 등)

	// 1. 슬롯 보관 시도
	if (TargetProperty->TryStoreItem(SlotTag, ItemToStore))
	{
		// 1. 상태 변경 (물리 비활성화)
		ItemManager->StoreItem(ItemToStore->GetInstanceId());

		// 2. 부착
		UInteractableComponent* SnapTarget = TargetProperty->GetOwner()
			? TargetProperty->GetOwner()->FindComponentByClass<UInteractableComponent>()
			: nullptr;
		TargetProperty->AttachTargetItem(ItemToStore, SnapTarget);

		// 3. 수동 부착 복제 정보 갱신
		if (UItemStateComponent* StateComp = ItemToStore->GetStateComponent())
		{
			StateComp->UpdateAttachmentReplication();
		}

		return true;
	}

	return false;
}

bool UInteractionManager::SafeStoreTransferItem(UInteractablePropertyComponent* SourceProperty, FGameplayTag SourceSlot, UInteractablePropertyComponent* TargetProperty, FGameplayTag TargetSlot)
{
	if (!SourceProperty || !TargetProperty) return false;

	AItemBase* ItemToMove = SourceProperty->GetStoredItem(SourceSlot);
	if (!ItemToMove) return false;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return false;

	// 1. 원본 저장소에서 해제
	SourceProperty->RetrieveItem(SourceSlot);
	SourceProperty->DetachTargetItem(ItemToMove);

	// 2. 대상 저장소에 보관
	if (TargetProperty->TryStoreItem(TargetSlot, ItemToMove))
	{
		// 상태 관리는 이미 Stored이므로 물리적 부착만 수행
		UInteractableComponent* SnapTarget = TargetProperty->GetOwner()
			? TargetProperty->GetOwner()->FindComponentByClass<UInteractableComponent>()
			: nullptr;
		TargetProperty->AttachTargetItem(ItemToMove, SnapTarget);

		// 3. 수동 부착 복제 정보 갱신
		if (UItemStateComponent* StateComp = ItemToMove->GetStateComponent())
		{
			StateComp->UpdateAttachmentReplication();
		}
		return true;
	}

	return false;
}

bool UInteractionManager::SafeStoreItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag)
{
	// 하위 호환성을 위해 SafeStoreHandItem 호출
	return SafeStoreHandItem(InteractorProperty, TargetProperty, ItemToStore, SlotTag);
}

bool UInteractionManager::SafeRetrieveItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToRetrieve, FGameplayTag SlotTag)
{
	if (!InteractorProperty || !TargetProperty || !ItemToRetrieve) return false;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return false;

	// 1. 슬롯에서 아이템 해제
	AItemBase* RetrievedItem = TargetProperty->RetrieveItem(SlotTag);
	if (RetrievedItem == ItemToRetrieve)
	{
		// 2. 물리적 분리
		TargetProperty->DetachTargetItem(ItemToRetrieve);

		// 3. 인터랙터에 물리적 부착
		InteractorProperty->ForceEquip(ItemToRetrieve);
		
		// 4. 논리적 상태 갱신
		ItemManager->RetrieveItem(ItemToRetrieve->GetInstanceId());

		// 5. 수동 부착 복제 정보 갱신
		if (UItemStateComponent* StateComp = ItemToRetrieve->GetStateComponent())
		{
			StateComp->UpdateAttachmentReplication();
		}
		return true;
	}

	return false;
}

AItemBase* UInteractionManager::SafeSpawnItem(FGameplayTag ItemTag, const FTransform& SpawnTransform)
{
	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return nullptr;

	FGuid NewItemId = ItemManager->SpawnItem(ItemTag, SpawnTransform);
	if (NewItemId.IsValid())
	{
		return ItemManager->GetItemActor(NewItemId);
	}
	return nullptr;
}

void UInteractionManager::SafeDestroyItem(AItemBase* Item)
{
	if (!Item) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	ItemManager->DestroyItem(Item->GetInstanceId());
}

bool UInteractionManager::SafeAttachItemToSlot(AItemBase* Item, UInteractablePropertyComponent* TargetProperty, FGameplayTag SlotTag)
{
	if (!Item || !TargetProperty) return false;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return false;

	// 1. 논리적 스토어 시도 및 부착
	if (TargetProperty->TryStoreItem(SlotTag, Item))
	{
		// 1. 상태 변경 (물리 비활성화)
		ItemManager->StoreItem(Item->GetInstanceId());

		// 2. 부착
		UInteractableComponent* SnapTarget = TargetProperty->GetOwner()
			? TargetProperty->GetOwner()->FindComponentByClass<UInteractableComponent>()
			: nullptr;
		TargetProperty->AttachTargetItem(Item, SnapTarget);
		
		// 3. 수동 부착 복제 정보 갱신
		if (UItemStateComponent* StateComp = Item->GetStateComponent())
		{
			StateComp->UpdateAttachmentReplication();
		}

		return true;
	}

	return false;
}

AItemBase* UInteractionManager::SafeDetachItemFromSlot(UInteractablePropertyComponent* TargetProperty, FGameplayTag SlotTag)
{
	if (!TargetProperty) return nullptr;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return nullptr;

	// 1. 논리적 추출 및 물리적 분리
	AItemBase* Item = TargetProperty->RetrieveItem(SlotTag);
	if (Item)
	{
		TargetProperty->DetachTargetItem(Item);
		
		// 2. 관리 시스템 상태 갱신
		ItemManager->RetrieveItem(Item->GetInstanceId());
		return Item;
	}

	return nullptr;
}

void UInteractionManager::SafeUpdateItemStat(AItemBase* Item, FGameplayTag StatTag, float NewValue)
{
	if (!Item || !StatTag.IsValid()) return;

	if (ILogicContextInterface* Context = Cast<ILogicContextInterface>(Item))
	{
		Context->SetStat(StatTag, FItemStatValue::MakeFloat(NewValue));
	}
}

void UInteractionManager::SafeUpdateProgressUI(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentTag, FGameplayTag MaxTag, float Current, float Max, FGameplayTag SlotTag)
{
	// 기존 UpdateStepProgress 기능을 래핑하여 사용
	UpdateStepProgress(TargetProperty, CurrentTag, MaxTag, Current, Max, SlotTag);
}

void UInteractionManager::ExecuteEquip(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToEquip)
{
	SafePickUpItem(InteractorProperty, ItemToEquip);
}

void UInteractionManager::ExecuteDrop(UInteractorPropertyComponent* InteractorProperty, AItemBase* CarriedItem, FVector OptionalImpulse)
{
	SafeDropItem(InteractorProperty, CarriedItem, OptionalImpulse);
}

void UInteractionManager::ExecuteStore(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag)
{
	SafeStoreItem(InteractorProperty, TargetProperty, ItemToStore, SlotTag);
}

void UInteractionManager::ExecuteRetrieve(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToRetrieve, FGameplayTag SlotTag)
{
	SafeRetrieveItem(InteractorProperty, TargetProperty, ItemToRetrieve, SlotTag);
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
			// 4. 즉시 Result Item 보관
			if (TargetProperty->TryStoreItem(TargetSlotTag, NewItem))
			{
				// 1. 상태 변경 먼저 (물리 비활성화)
				ItemManager->StoreItem(NewItemId);

				// 2. 부착
				UInteractableComponent* SnapTarget = TargetActor
					? TargetActor->FindComponentByClass<UInteractableComponent>()
					: nullptr;
				TargetProperty->AttachTargetItem(NewItem, SnapTarget);

				// 3. 수동 부착 복제 정보 갱신
				if (UItemStateComponent* StateComp = NewItem->GetStateComponent())
				{
					StateComp->UpdateAttachmentReplication();
				}
			}
		}
	}
}

void UInteractionManager::ExecuteTransformItem(UInteractablePropertyComponent* TargetProperty, AItemBase* OriginalItem, FGameplayTag ResultItemTag, FGameplayTag TargetSlotTag)
{
	if (!TargetProperty || !OriginalItem) return;

	UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
	if (!ItemManager) return;

	// 1. 기존 아이템을 물리적/시각적으로 분리 및 보관 해제
	if (TargetProperty->RetrieveItem(TargetSlotTag) == OriginalItem)
	{
		TargetProperty->DetachTargetItem(OriginalItem);
	}

	// 2. 기존 아이템 파괴 요청
	ItemManager->DestroyItem(OriginalItem->GetInstanceId());

	// 3. 결과 아이템 생성 요청
	AActor* TargetActor = TargetProperty->GetOwner();
	FTransform SpawnTransform = TargetActor ? TargetActor->GetActorTransform() : FTransform::Identity;
	FGuid NewItemId = ItemManager->SpawnItem(ResultItemTag, SpawnTransform);

	if (NewItemId.IsValid())
	{
		AItemBase* NewItem = ItemManager->GetItemActor(NewItemId);
		if (NewItem)
		{
			// 4. 새로운 아이템을 즉시 보관 및 부착
			if (TargetProperty->TryStoreItem(TargetSlotTag, NewItem))
			{
				// 1. 상태 변경 먼저 (물리 비활성화)
				ItemManager->StoreItem(NewItemId);

				// 2. 부착
				UInteractableComponent* SnapTarget = TargetActor
					? TargetActor->FindComponentByClass<UInteractableComponent>()
					: nullptr;
				TargetProperty->AttachTargetItem(NewItem, SnapTarget);

				// 3. 수동 부착 복제 정보 갱신
				if (UItemStateComponent* StateComp = NewItem->GetStateComponent())
				{
					StateComp->UpdateAttachmentReplication();
				}
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

void UInteractionManager::StartHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, float RequiredDuration, FGameplayTag SlotTag, FGuid ItemUID, float InitialProgress)
{
	if (!TargetProperty || !TargetProperty->GetOwner()) return;

	UE_LOG(LogTemp, Log, TEXT("[InteractionManager] StartHoldingProgress - Target: %s, SlotTag: %s, ItemUID: %s, InitialProgress: %f"), 
		*TargetProperty->GetOwner()->GetName(), *SlotTag.ToString(), *ItemUID.ToString(), InitialProgress);

	float CurrentTime = 0.0f;
	if (AGameStateBase* GS = TargetProperty->GetWorld()->GetGameState())
	{
		CurrentTime = GS->GetServerWorldTimeSeconds();
	}
	else
	{
		CurrentTime = TargetProperty->GetWorld()->GetTimeSeconds();
	}

	float EndTime = CurrentTime + (RequiredDuration - InitialProgress);
	float StartTime = CurrentTime - InitialProgress;

	FItemStatValue StartStat;
	StartStat.Type = EItemStatType::Float;
	StartStat.FloatValue = StartTime;

	FItemStatValue EndStat;
	EndStat.Type = EItemStatType::Float;
	EndStat.FloatValue = EndTime;

	// 1. 데이터 저장 (Workstation 또는 Item)
	ILogicContextInterface* StatTarget = nullptr;
	UInteractablePropertyComponent* UITarget = TargetProperty;

	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			StatTarget = Cast<ILogicContextInterface>(StoredItem);
			UITarget = StoredItem->FindComponentByClass<UInteractablePropertyComponent>();
			UE_LOG(LogTemp, Log, TEXT("[InteractionManager] Redirection Success - StoredItem: %s, UITarget: %s"), 
				*StoredItem->GetName(), UITarget ? *UITarget->GetName() : TEXT("None"));
		}
	}
	
	if (!UITarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				StatTarget = Cast<ILogicContextInterface>(ItemActor);
				UITarget = ItemActor->FindComponentByClass<UInteractablePropertyComponent>();
				UE_LOG(LogTemp, Log, TEXT("[InteractionManager] Redirection via ItemUID - Actor: %s"), 
					*ItemActor->GetName());
			}
		}
	}
	
	if (!StatTarget)
	{
		StatTarget = Cast<ILogicContextInterface>(TargetProperty->GetOwner());
	}

	if (StatTarget)
	{
		StatTarget->SetStat(StartTimeTag, StartStat);
		StatTarget->SetStat(EndTimeTag, EndStat);
	}

	// 2. UI 표시 (InternalSetTimerUI 호출)
	if (UITarget)
	{
		UITarget->InternalSetTimerUI(StartTimeTag, EndTimeTag, ItemUID);
	}
}

void UInteractionManager::ClearHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, FGameplayTag SlotTag, FGuid ItemUID)
{
	if (!TargetProperty || !TargetProperty->GetOwner()) return;

	UE_LOG(LogTemp, Log, TEXT("[InteractionManager] ClearHoldingProgress - Target: %s, SlotTag: %s, ItemUID: %s"), 
		*TargetProperty->GetOwner()->GetName(), *SlotTag.ToString(), *ItemUID.ToString());

	FItemStatValue ClearStat;
	ClearStat.Type = EItemStatType::Float;
	ClearStat.FloatValue = -1.0f;

	// 1. 데이터 초기화 (도메인 모델의 스탯 초기화)
	ILogicContextInterface* StatTarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			StatTarget = Cast<ILogicContextInterface>(StoredItem);
		}
	}
	
	if (!StatTarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				StatTarget = Cast<ILogicContextInterface>(ItemActor);
			}
		}
	}

	if (!StatTarget)
	{
		StatTarget = Cast<ILogicContextInterface>(TargetProperty->GetOwner());
	}

	if (StatTarget)
	{
		StatTarget->SetStat(StartTimeTag, ClearStat);
		StatTarget->SetStat(EndTimeTag, ClearStat);
	}

	// 2. UI 상태 초기화 (InternalClearUI 호출)
	UInteractablePropertyComponent* UITarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			UITarget = StoredItem->FindComponentByClass<UInteractablePropertyComponent>();
		}
	}

	if (!UITarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				UITarget = ItemActor->FindComponentByClass<UInteractablePropertyComponent>();
				UE_LOG(LogTemp, Log, TEXT("[InteractionManager] ClearHoldingProgress - Redirection via ItemUID: %s"), 
					*ItemActor->GetName());
			}
		}
	}

	if (!UITarget)
	{
		UITarget = TargetProperty;
	}

	if (UITarget)
	{
		UITarget->InternalClearUI();
	}
}

void UInteractionManager::FreezeHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, FGameplayTag SlotTag, FGuid ItemUID)
{
	if (!TargetProperty) return;

	UInteractablePropertyComponent* UITarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			UITarget = StoredItem->FindComponentByClass<UInteractablePropertyComponent>();
		}
	}

	if (!UITarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				UITarget = ItemActor->FindComponentByClass<UInteractablePropertyComponent>();
				UE_LOG(LogTemp, Log, TEXT("[InteractionManager] FreezeHoldingProgress - Redirection via ItemUID: %s"), 
					*ItemActor->GetName());
			}
		}
	}

	if (!UITarget)
	{
		UITarget = TargetProperty;
	}

	// 컴포넌트 내부 헬퍼 호출 (원자적으로 타이머 → 스텝 상태 전환)
	if (UITarget)
	{
		UITarget->InternalFreezeTimerToStep(CurrentStepTag, MaxStepTag);
	}
}

void UInteractionManager::UpdateStepProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, float CurrentStep, float MaxStep, FGameplayTag SlotTag, FGuid ItemUID)
{
	if (!TargetProperty) return;

	UInteractablePropertyComponent* UITarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			UITarget = StoredItem->FindComponentByClass<UInteractablePropertyComponent>();
		}
	}

	if (!UITarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				UITarget = ItemActor->FindComponentByClass<UInteractablePropertyComponent>();
				UE_LOG(LogTemp, Log, TEXT("[InteractionManager] UpdateStepProgress - Redirection via ItemUID: %s"), 
					*ItemActor->GetName());
			}
		}
	}

	if (!UITarget)
	{
		UITarget = TargetProperty;
	}

	// 컴포넌트 내부 헬퍼 호출
	if (UITarget)
	{
		UITarget->InternalSetStepUI(CurrentStepTag, MaxStepTag, CurrentStep, MaxStep, ItemUID);
	}
}

void UInteractionManager::ClearStepProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, FGameplayTag SlotTag, FGuid ItemUID)
{
	FItemStatValue ClearStat;
	ClearStat.Type = EItemStatType::Float;
	ClearStat.FloatValue = -1.0f;

	// 1. 데이터 초기화 (Blackboard Stat)
	ILogicContextInterface* StatTarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			StatTarget = Cast<ILogicContextInterface>(StoredItem);
		}
	}

	if (!StatTarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				StatTarget = Cast<ILogicContextInterface>(ItemActor);
			}
		}
	}

	if (!StatTarget)
	{
		StatTarget = Cast<ILogicContextInterface>(TargetProperty->GetOwner());
	}

	if (StatTarget)
	{
		StatTarget->SetStat(CurrentStepTag, ClearStat);
		StatTarget->SetStat(MaxStepTag, ClearStat);
	}

	// 컴포넌트 UI 초기화
	UInteractablePropertyComponent* UITarget = nullptr;
	if (SlotTag.IsValid())
	{
		if (AItemBase* StoredItem = TargetProperty->GetStoredItem(SlotTag))
		{
			UITarget = StoredItem->FindComponentByClass<UInteractablePropertyComponent>();
		}
	}

	if (!UITarget && ItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AItemBase* ItemActor = ItemManager->GetItemActor(ItemUID))
			{
				UITarget = ItemActor->FindComponentByClass<UInteractablePropertyComponent>();
				UE_LOG(LogTemp, Log, TEXT("[InteractionManager] ClearStepProgress - Redirection via ItemUID: %s"), 
					*ItemActor->GetName());
			}
		}
	}

	if (!UITarget)
	{
		UITarget = TargetProperty;
	}

	if (UITarget)
	{
		UITarget->InternalClearUI();
	}
}

// Redundant UI helpers removed. Managed by InteractablePropertyComponent directly.

