#include "LKH2/Interaction/Logic/Logic_Interactable_Bake.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"

ULogic_Interactable_Bake::ULogic_Interactable_Bake()
{
	// 기본값 설정
	TargetSlotTag = FGameplayTag::RequestGameplayTag(TEXT("Slot.Main"));
	BakeActionTag = FGameplayTag::RequestGameplayTag(TEXT("Interaction.Action.Bake"));

	// 시작/중단 의도 설정 (상태 컴포넌트의 가공 의도와 정렬)
	StartIntentTag = FGameplayTag::RequestGameplayTag(TEXT("Intent.Workstation.ItemAdd"));
	StopIntentTag  = FGameplayTag::RequestGameplayTag(TEXT("Intent.Workstation.ItemRemove"));

	// UI 설정: 아이템 위에 표시하고 취소 시에도 유지 (진행도 동결)
	UISlotTag = TargetSlotTag;
	bMaintainUIOnCancel = true;
	CurrentStepTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.Interaction.Progress"));
	MaxStepTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.Interaction.MaxProgress"));
}

bool ULogic_Interactable_Bake::CanStartProcessing() const
{
	// 1. 기본 조건 체크 (아이템 개수 등)
	if (!Super::CanStartProcessing()) return false;

	UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>();
	ULogicContextComponent* ContextComp = OwnerActor->FindComponentByClass<ULogicContextComponent>();
	if (!PropComp) return false;

	// 2. 구울 아이템 찾기 (Container 대응: GetTargetItemDeep 사용)
	FGameplayTag ResolvedSlotTag = ContextComp ? ContextComp->ResolveKey(TargetSlotTag) : TargetSlotTag;
	AItemBase* TargetItem = GetTargetItemDeep(FInteractionContext(), PropComp, ResolvedSlotTag);

	if (!TargetItem || !TargetItem->GetItemData())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] CanStartProcessing - No valid target item for processing in slot [%s]."), *GetName(), *ResolvedSlotTag.ToString());
		return false;
	}

	// 3. 레시피 존재 여부 확인
	if (UItemRecipeManager* RecipeManager = GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>())
	{
		FGameplayTag InputTag = TargetItem->GetItemData()->ItemTag;
		TArray<FGameplayTag> Results = RecipeManager->GetRecipeResultSingle(InputTag, BakeActionTag);
		
		if (Results.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] CanStartProcessing - Valid recipe found for [%s]. Deep detection: %s"), 
				*GetName(), *InputTag.ToString(), *TargetItem->GetName());
			return true;
		}
	}

	return false;
}

void ULogic_Interactable_Bake::OnProcessingCompleted_Implementation(const FInteractionContext& Context)
{
	if (!OwnerActor) return;

	UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>();
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!PropComp) return;

	// 1. 해당 슬롯에 있는 실제 가공 대상 확인 (Container 내부 탐색)
	FGameplayTag ResolvedSlotTag = ContextComp ? ContextComp->ResolveKey(TargetSlotTag) : TargetSlotTag;
	AItemBase* OriginalItem = GetTargetItemDeep(Context, PropComp, ResolvedSlotTag);

	if (!OriginalItem || !OriginalItem->GetItemData())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnProcessingCompleted - No valid item found in slot [%s]."), *GetName(), *ResolvedSlotTag.ToString());
		return;
	}

	FGameplayTag InputTag = OriginalItem->GetItemData()->ItemTag;
	FGameplayTag ResultTag = FallbackResultItemTag;

	// 2. RecipeManager를 통해 결과물 조회
	if (UItemRecipeManager* RecipeManager = GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>())
	{
		TArray<FGameplayTag> RecipeResults = RecipeManager->GetRecipeResultSingle(InputTag, BakeActionTag);
		if (RecipeResults.Num() > 0)
		{
			ResultTag = RecipeResults[0];
		}
	}

	if (!ResultTag.IsValid()) return;

	// 3. InteractionManager를 통해 아이템 변환 실행
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		// 겉표면 아이템과 실제 가공 대상이 다르다면 (컨테이너 내부라면)
		AItemBase* SlotItem = PropComp->GetStoredItem(ResolvedSlotTag);
		
		UInteractablePropertyComponent* TargetPropComp = PropComp;
		FGameplayTag ActualItemSlot = ResolvedSlotTag;

		if (SlotItem && OriginalItem && SlotItem != OriginalItem)
		{
			// 컨테이너 내부 가공인 경우
			if (UInteractablePropertyComponent* ContainerProp = SlotItem->FindComponentByClass<UInteractablePropertyComponent>())
			{
				TargetPropComp = ContainerProp;
				// 컨테이너 내부의 어느 슬롯에 있는지 찾음
				for (auto& Pair : ContainerProp->StoredItems)
				{
					if (Pair.Value == OriginalItem)
					{
						ActualItemSlot = Pair.Key;
						break;
					}
				}
				UE_LOG(LogTemp, Log, TEXT("[%s] Container Internal Processing Detected. Container: %s, Slot: %s"), 
					*GetName(), *SlotItem->GetName(), *ActualItemSlot.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Direct Processing Detected on Slot: %s"), 
				*GetName(), *ResolvedSlotTag.ToString());
		}

		IM->ExecuteTransformItem(TargetPropComp, OriginalItem, ResultTag, ActualItemSlot);
	}
}
