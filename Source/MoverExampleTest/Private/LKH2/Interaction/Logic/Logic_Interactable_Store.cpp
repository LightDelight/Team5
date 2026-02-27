// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Interactable_Store.h"

ULogic_Interactable_Store::ULogic_Interactable_Store()
{
}

bool ULogic_Interactable_Store::PreInteractCheck(const FInteractionContext &Context)
{
	if (RequiredIntentTag.IsValid() && !Context.InteractionTag.MatchesTag(RequiredIntentTag))
	{
		return false;
	}

	// TODO: 저장(Store)이 가능한 조건인지 확인하는 추가 로직

	return true;
}

bool ULogic_Interactable_Store::PerformInteraction(const FInteractionContext &Context)
{
	// TODO: 실제 Store 상호작용에 필요한 핵심 동작 수행
	// InteractionManager->ExecuteStore(...) 와 같은 호출이 들어갈 예정

	return true;
}
