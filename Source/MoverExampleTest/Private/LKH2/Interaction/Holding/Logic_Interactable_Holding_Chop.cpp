// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Holding/Logic_Interactable_Holding_Chop.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKh2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ULogic_Interactable_Holding_Chop::ULogic_Interactable_Holding_Chop()
{
	// 기본값 설정 (에디터에서 덮어쓰기 가능)
	RecipeTag         = FGameplayTag::RequestGameplayTag(TEXT("Interaction.Chop"));
	SlotTag           = FGameplayTag::RequestGameplayTag(TEXT("Slot.ChoiceBoard.Primary"));
	ActionTag         = FGameplayTag::RequestGameplayTag(TEXT("Action.Chop"));
	RequiredIntentTag = FGameplayTag::RequestGameplayTag(TEXT("Intent.Holding.Chop"));
	ProgressIntentTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Montage.Hit"));
	CancelIntentTag   = FGameplayTag::RequestGameplayTag(TEXT("Intent.Cancel"));
	MaxStep           = 5.0f;
}

void ULogic_Interactable_Holding_Chop::OnHoldingCompleted_Implementation(const FInteractionContext& Context)
{
	Super::OnHoldingCompleted_Implementation(Context);

	UWorld* World = GetWorld();
	if (!World) return;

	UInteractionManager* InteractionManager = World->GetSubsystem<UInteractionManager>();
	UItemRecipeManager* RecipeManager = World->GetGameInstance()->GetSubsystem<UItemRecipeManager>();
	if (!InteractionManager || !RecipeManager) return;

	UInteractablePropertyComponent* TargetProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	if (!TargetProp) return;

	// 1. 현재 슬롯의 아이템 확인
	AItemBase* StoredItem = TargetProp->GetStoredItem(SlotTag);
	if (!StoredItem || !StoredItem->GetItemData()) return;

	// 2. 레시피 결과 조회
	TArray<FGameplayTag> Inputs = { StoredItem->GetItemData()->ItemTag };
	TArray<FGameplayTag> Results = RecipeManager->GetRecipeResultMulti(Inputs, RecipeTag);

	if (Results.Num() > 0)
	{
		FGameplayTag ResultTag = Results[0];
		UE_LOG(LogTemp, Log, TEXT("[ChopLogic] 요리 완료! 결과물: %s"), *ResultTag.ToString());
		
		// 3. 기존 아이템 슬롯에서 분리 후 파괴
		// SafeDestroyItem만 호출하면 PropertyComponent의 StoredItems 슬롯 맵이 정리되지 않아
		// 이후 SafeAttachItemToSlot의 TryStoreItem이 슬롯 점유로 인해 실패함.
		InteractionManager->SafeDetachItemFromSlot(TargetProp, SlotTag); // 슬롯 맵 정리
		InteractionManager->SafeDestroyItem(StoredItem);                 // 아이템 파괴

		// 4. 새 결과 아이템 생성 및 슬롯 부착 (Atomic Safe API)
		AActor* ContextOwner = TargetProp->GetOwner();
		FTransform SpawnTransform = ContextOwner ? ContextOwner->GetActorTransform() : FTransform::Identity;
		
		AItemBase* NewItem = InteractionManager->SafeSpawnItem(ResultTag, SpawnTransform);
		if (NewItem)
		{
			InteractionManager->SafeAttachItemToSlot(NewItem, TargetProp, SlotTag);
			UE_LOG(LogTemp, Log, TEXT("[ChopLogic] 결과물 아이템 부착 성공."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChopLogic] 레시피 결과를 찾을 수 없습니다. (Inputs: %s, Recipe: %s)"), 
			*StoredItem->GetItemData()->ItemTag.ToString(), *RecipeTag.ToString());
	}

	// 5. 진행도 UI 및 데이터 초기화 (Atomic Safe API)
	InteractionManager->ClearStepProgress(TargetProp, ProgressStatTag, MaxProgressStatTag, SlotTag);
}
