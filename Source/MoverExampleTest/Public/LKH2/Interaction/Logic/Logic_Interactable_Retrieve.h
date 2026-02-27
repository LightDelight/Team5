// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Retrieve.generated.h"

/**
 * 워크스테이션에서 아이템을 회수(Retrieve)하는 상호작용 로직의 뼈대
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Retrieve Item"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Retrieve : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	ULogic_Interactable_Retrieve();

	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;

	// 꺼내올 슬롯 태그 (예: Slot.Left)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Retrieve")
	FGameplayTag SlotTag;
};
