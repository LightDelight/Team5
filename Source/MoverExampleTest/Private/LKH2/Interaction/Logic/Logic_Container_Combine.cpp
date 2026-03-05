// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Container_Combine.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/GameInstance.h"

bool ULogic_Container_Combine::PreInteractCheck(const FInteractionContext &Context)
{
	if (RecipeInteractionTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck: InteractionTag 미일치 (%s != %s)"), *Context.InteractionTag.ToString(), *RecipeInteractionTag.ToString());
		return false;
	}

	UInteractablePropertyComponent* ContainerProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	if (!ContainerProperty)
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck 실패: ContainerProperty 없음"));
		return false;
	}

	if (ContainerProperty->StoredItems.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck: 컨테이너가 비어있음"));
		return false;
	}

	// 1. Container 내 모든 아이템 태그 수집
	TArray<FGameplayTag> InputTags;
	for (auto& Elem : ContainerProperty->StoredItems)
	{
		if (AItemBase* Item = Elem.Value)
		{
			if (UItemData* ItemData = Item->GetItemData())
			{
				InputTags.Add(ItemData->ItemTag);
			}
		}
	}

	// 1-2. 상호작용 대상(TargetActor)이 아이템이면 재료에 추가
	if (AItemBase* TargetItem = Cast<AItemBase>(Context.TargetActor))
	{
		if (UItemData* ItemData = TargetItem->GetItemData())
		{
			InputTags.Add(ItemData->ItemTag);
		}
	}

	if (InputTags.IsEmpty()) 
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck: 유효한 아이템 태그를 찾지 못함"));
		return false;
	}

	// 2. 레시피 매니저 쿼리
	UItemRecipeManager* RecipeManager = Context.Interactor->GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>();
	if (!RecipeManager) 
	{
		UE_LOG(LogTemp, Error, TEXT("[Container_Combine] PreCheck 실패: RecipeManager 없음"));
		return false;
	}

	TArray<FGameplayTag> Results = RecipeManager->GetRecipeResultMulti(InputTags, RecipeInteractionTag);
	
	if (Results.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck: 일치하는 레시피 없음 (입력 개수: %d)"), InputTags.Num());
		return false;
	}

	FString InputNames;
	for (const FGameplayTag& Tag : InputTags) { InputNames += Tag.ToString() + TEXT(", "); }
	UE_LOG(LogTemp, Log, TEXT("[Container_Combine] PreCheck 성공: 입력[%s] -> 결과[%s] 조합 가능"), 
		*InputNames, *Results[0].ToString());

	return true;
}

bool ULogic_Container_Combine::PerformInteraction(const FInteractionContext &Context)
{
	UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
	if (!InteractionManager) 
	{
		UE_LOG(LogTemp, Error, TEXT("[Container_Combine] Perform 실패: InteractionManager 없음"));
		return false;
	}

	UInteractablePropertyComponent* ContainerProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	if (!ContainerProperty) 
	{
		UE_LOG(LogTemp, Error, TEXT("[Container_Combine] Perform 실패: ContainerProperty 없음"));
		return false;
	}

	// 1. 다시 한번 태그 수집 및 레시피 확인
	TArray<FGameplayTag> InputTags;
	TArray<FGameplayTag> SlotKeys;
	ContainerProperty->StoredItems.GetKeys(SlotKeys);

	for (const FGameplayTag& SlotTag : SlotKeys)
	{
		if (AItemBase* Item = ContainerProperty->GetStoredItem(SlotTag))
		{
			if (Item->GetItemData())
			{
				InputTags.Add(Item->GetItemData()->ItemTag);
			}
		}
	}

	// 1-2. 외부 대상 아이템 추가
	AItemBase* TargetItem = Cast<AItemBase>(Context.TargetActor);
	if (TargetItem && TargetItem->GetItemData())
	{
		InputTags.Add(TargetItem->GetItemData()->ItemTag);
	}

	UItemRecipeManager* RecipeManager = GetWorld()->GetGameInstance()->GetSubsystem<UItemRecipeManager>();
	TArray<FGameplayTag> Results = RecipeManager->GetRecipeResultMulti(InputTags, RecipeInteractionTag);

	if (Results.IsEmpty()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Container_Combine] Perform 실패: 최종 레시피 확인 실패"));
		return false;
	}

	FGameplayTag ResultTag = Results[0];
	UE_LOG(LogTemp, Log, TEXT("[Container_Combine] 조합 시작: 결과[%s]를 위해 [%d]개 재료 소모 시퀀스 진입"), 
		*ResultTag.ToString(), InputTags.Num());

	// 2. 모든 재료 아이템 파괴 (컨테이너 내부)
	for (const FGameplayTag& SlotTag : SlotKeys)
	{
		if (AItemBase* Item = ContainerProperty->GetStoredItem(SlotTag))
		{
			ContainerProperty->RetrieveItem(SlotTag);
			ContainerProperty->DetachTargetItem(Item);
			InteractionManager->SafeDestroyItem(Item);
		}
	}

	// 2-2. 외부 재료 아이템 파괴
	if (TargetItem)
	{
		// 만약 다른 컴포넌트에 부착되어 있었다면 해제 시도 (예: 작업대 위에 놓인 아이템 상호작용 시)
		if (AActor* Parent = TargetItem->GetAttachParentActor())
		{
			if (UInteractablePropertyComponent* ParentProp = Parent->FindComponentByClass<UInteractablePropertyComponent>())
			{
				// 슬롯은 알 수 없으므로 전체 슬롯을 순회하여 해당 아이템이 있는지 찾아 해제
				for (const auto& Elem : ParentProp->StoredItems)
				{
					if (Elem.Value == TargetItem)
					{
						ParentProp->RetrieveItem(Elem.Key);
						break;
					}
				}
				ParentProp->DetachTargetItem(TargetItem);
			}
		}
		InteractionManager->SafeDestroyItem(TargetItem);
	}

	// 3. 결과 아이템 생성하여 Container에 보관
	AItemBase* ResultItem = InteractionManager->SafeSpawnItem(ResultTag, ContainerProperty->GetComponentTransform());
	if (ResultItem)
	{
		UE_LOG(LogTemp, Log, TEXT("[Container_Combine] 결과물 생성 성공: %s"), *ResultItem->GetName());
		InteractionManager->SafeStoreWorldItem(ContainerProperty, ResultItem, ResultSlotTag);

		UE_LOG(LogTemp, Warning, TEXT("[Container_Combine] 조합 실행 완료: 결과물[%s] 생성 및 [%s] 슬롯 보관"), 
			*ResultTag.ToString(), *ResultSlotTag.ToString());
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("[Container_Combine] Perform 실패: 결과 아이템(%s) 생성 실패"), *ResultTag.ToString());
	return false;
}
