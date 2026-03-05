// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Container_Clear.generated.h"

/**
 * 상호작용 시 블랙보드 데이터를 초기화하거나 특정 키를 삭제하는 로직 모듈입니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Container Logic: Clear Blackboard"))
class MOVEREXAMPLETEST_API ULogic_Container_Clear : public ULogicModuleBase
{
	GENERATED_BODY()

protected:
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
	/** 삭제할 스탯 태그 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	TArray<FGameplayTag> StatTagsToClear;

	/** 삭제할 오브젝트 태그 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	TArray<FGameplayTag> ObjectTagsToClear;

	/** 모든 스탯을 지울지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	bool bClearAllStats = false;

	/** 모든 오브젝트를 지울지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	bool bClearAllObjects = false;

	/** 프로그레스 UI도 숨길지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear")
	bool bClearProgressUI = false;
};
