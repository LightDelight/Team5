// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Store.generated.h"

/**
 * 아이템을 저장(Store)하는 상호작용 로직의 뼈대
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Store Item"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Store : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	ULogic_Interactable_Store();

	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
