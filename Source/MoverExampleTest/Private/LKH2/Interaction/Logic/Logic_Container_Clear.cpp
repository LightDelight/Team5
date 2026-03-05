// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Container_Clear.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"

bool ULogic_Container_Clear::PreInteractCheck(const FInteractionContext &Context)
{
	// 에디터에서 설정한 의도 태그가 일치하는지 확인
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	return true;
}

bool ULogic_Container_Clear::PerformInteraction(const FInteractionContext &Context)
{
	UInteractablePropertyComponent* TargetProperty = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
	if (!TargetProperty)
	{
		return false;
	}

	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp)
	{
		return false;
	}

	bool bChanged = false;

	UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG] 🧹 블랙보드 정리 시작: 액터=%s, Intent=%s"), 
		*TargetProperty->GetOwner()->GetName(), *RequiredIntentTag.ToString());

	// 1. 스탯 처리
	if (bClearAllStats)
	{
		ContextComp->ClearAllStats();
		UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG]   - 모든 스탯 초기화 완료"));
		bChanged = true;
	}
	else if (StatTagsToClear.Num() > 0)
	{
		for (const FGameplayTag& Tag : StatTagsToClear)
		{
			ContextComp->RemoveStat(Tag);
			UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG]   - 스탯 삭제: %s"), *Tag.ToString());
			bChanged = true;
		}
	}

	// 2. 오브젝트 처리
	if (bClearAllObjects)
	{
		ContextComp->ClearAllObjects();
		UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG]   - 모든 오브젝트 초기화 완료"));
		bChanged = true;
	}
	else if (ObjectTagsToClear.Num() > 0)
	{
		for (const FGameplayTag& Tag : ObjectTagsToClear)
		{
			ContextComp->RemoveObject(Tag);
			UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG]   - 오브젝트 삭제: %s"), *Tag.ToString());
			bChanged = true;
		}
	}

	if (bClearProgressUI)
	{
		TargetProperty->HideProgressUI();
		UE_LOG(LogTemp, Warning, TEXT("[Logic_Container_Clear_DEBUG]   - 프로그레스 UI 숨김 처리 완료"));
		bChanged = true;
	}

	if (!bChanged)
	{
		UE_LOG(LogTemp, Log, TEXT("[Logic_Container_Clear_DEBUG]   - 삭제할 항목이 없어 아무 작업도 하지 않았습니다."));
	}

	return true;
}
