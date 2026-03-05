// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Throw.generated.h"

/**
 * 아이템을 던지는 상호작용 로직
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Logic: Throw Item"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Throw : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 이 로직은 아이템을 들고 있는 상태에서만 유효하게 동작해야 함.
	virtual bool PreInteractCheck(const FInteractionContext& Context) override;
	virtual bool PerformInteraction(const FInteractionContext& Context) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throw")
	float ThrowForce = 1000.0f;
};
