// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Holding/Logic_Interactable_Holding_Chop.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKh2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ULogic_Interactable_Holding_Chop::ULogic_Interactable_Holding_Chop()
{
	// 기본값 설정 (에디터에서 덮어쓰기 가능)
	RequiredItemTag   = FGameplayTag::EmptyTag; // 기본적으로 비워둠
	MaxStep           = 5.0f;
}

bool ULogic_Interactable_Holding_Chop::PreInteractCheck(const FInteractionContext& Context)
{
	// 1. 기본 홀딩 로직의 사전 검사 통과 여부 확인
	if (!Super::PreInteractCheck(Context))
	{
		return false;
	}

	// 2. 시작 의도(RequiredIntentTag)일 때만 필수 아이템 검사 수행
	if (Context.InteractionTag == RequiredIntentTag)
	{
		UInteractablePropertyComponent* TargetProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
		if (!TargetProp) return false;

		// 필수 아이템 태그가 지정되어 있다면, 플레이어가 들고 있는 아이템이 태그를 가지는지 확인
		if (RequiredItemTag.IsValid())
		{
			UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(Context.InteractorPropertyComp);
			if (!InteractorProp) return false;

			AItemBase* CarriedItem = Cast<AItemBase>(InteractorProp->GetCarriedActor());
			if (!CarriedItem || !CarriedItem->GetItemData()) return false;

			if (!CarriedItem->GetItemData()->ItemTag.MatchesTag(RequiredItemTag))
			{
				UE_LOG(LogTemp, Warning, TEXT("[ChopLogic] 필수 아이템 조건 불만족. (요구: %s, 현재: %s)"), 
					*RequiredItemTag.ToString(), *CarriedItem->GetItemData()->ItemTag.ToString());
				return false;
			}
		}
	}

	return true;
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
