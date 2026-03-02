// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Recipe_Combine.generated.h"

/**
 * 조합(Combine) 상호작용 관련 로직 모듈.
 * 워크스테이션 또는 아이템 슬롯(SlotTag)의 아이템과 상호작용자(Player)가 든 아이템을
 * 전역 UItemRecipeManager에 넘겨 조합 가능 여부를 체크하고 실행합니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Combine Recipe"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Recipe_Combine : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 레시피 매니저에 조합을 조회할 때 쓸 상호작용 태그 (예: Context.Action.Combine)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag RecipeInteractionTag;

	// 조합할 대상 아이템이 보관된 슬롯 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag SlotTag;

public:
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
