// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Vending.generated.h"

/**
 * 아이템을 구매하거나 뽑는(Vending) 상호작용 로직 모듈 스켈레톤입니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Interactable Logic: Vending"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Vending : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	/** 벤딩 머신에서 나올 아이템의 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Vending")
	FGameplayTag ItemToVend;

	// 상호작용 의도(Intent) 확인 및 가능한 상태인지 검사 (재화 확인 등)
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;

	// 실제 뽑기 연출 트리거 및 아이템 스폰 등 핵심 로직 수행
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
