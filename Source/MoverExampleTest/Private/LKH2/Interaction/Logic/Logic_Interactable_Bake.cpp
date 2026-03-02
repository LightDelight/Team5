#include "LKH2/Interaction/Logic/Logic_Interactable_Bake.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/World.h"

ULogic_Interactable_Bake::ULogic_Interactable_Bake()
{
	// 기본값 설정
	TargetSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Slot.Main"));
	BakeActionTag = FGameplayTag::RequestGameplayTag(TEXT("Interaction.Action.Bake"));
}

void ULogic_Interactable_Bake::OnProcessingCompleted_Implementation(const FInteractionContext& Context)
{
	if (!OwnerActor) return;

	UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>();
	if (!PropComp) return;

	// 1. 해당 슬롯에 있는 원본 아이템 확인
	AItemBase* OriginalItem = PropComp->GetStoredItem(TargetSlotTag);
	if (!OriginalItem || !OriginalItem->GetItemData()) return;

	FGameplayTag InputTag = OriginalItem->GetItemData()->ItemTag;
	FGameplayTag ResultTag = FallbackResultItemTag;

	UE_LOG(LogTemp, Log, TEXT("[%s] OnProcessingCompleted - Input Item: %s"), *GetName(), *InputTag.ToString());

	// 2. RecipeManager를 통해 결과물 조회
	if (UItemRecipeManager* RecipeManager = GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>())
	{
		TArray<FGameplayTag> RecipeResults = RecipeManager->GetRecipeResultSingle(InputTag, BakeActionTag);
		if (RecipeResults.Num() > 0)
		{
			ResultTag = RecipeResults[0];
			UE_LOG(LogTemp, Log, TEXT("[%s] Recipe Found: %s"), *GetName(), *ResultTag.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] No Recipe Found for %s with Action %s. Using Fallback: %s"), 
				*GetName(), *InputTag.ToString(), *BakeActionTag.ToString(), *ResultTag.ToString());
		}
	}

	if (!ResultTag.IsValid()) return;

	// 3. InteractionManager를 통해 아이템 변환 실행
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Executing Transform: %s -> %s"), *GetName(), *InputTag.ToString(), *ResultTag.ToString());
		IM->ExecuteTransformItem(PropComp, OriginalItem, ResultTag, TargetSlotTag);
	}
}
