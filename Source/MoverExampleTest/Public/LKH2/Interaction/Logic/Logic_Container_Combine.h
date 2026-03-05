// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Container_Combine.generated.h"

class AItemBase;
class UInteractablePropertyComponent;

/**
 * Container 내부의 아이템들을 조합(Combine)하는 로직 모듈.
 * Container의 모듈 배열에 등록됩니다.
 * 
 * Container에 담긴 모든 아이템 태그를 수집하여 ItemRecipeManager에 쿼리합니다.
 * 조합 결과가 있으면 기존 재료를 모두 파괴하고 결과 아이템을 Container의 첫 번째 슬롯에 생성합니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Container Combine"))
class MOVEREXAMPLETEST_API ULogic_Container_Combine : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 레시피 매니저에 조합을 조회할 때 쓸 상호작용 태그 (예: Context.Action.Combine)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag RecipeInteractionTag;

	// 결과 아이템을 생성할 슬롯 태그 (보통 첫 번째 슬롯)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag ResultSlotTag;

public:
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
