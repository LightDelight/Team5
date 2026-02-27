// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "HoldingLogicModuleBase.generated.h"

/**
 * 일정 시간 누르고 있어야 하는 지속성 상호작용 로직의 베이스 모듈입니다.
 * 
 * Holding(누르고 있기) 상호작용은 시작(Start), 진행(Tick), 성공(Complete), 취소(Cancel) 로 나뉠 수 있습니다.
 * 
 * TODO: 추후 Manager 쪽과 연계하여 Tick 갱신 및 UI 연동 등을 확장할 수 있습니다.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API UHoldingLogicModuleBase : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 생성자
	UHoldingLogicModuleBase();

	// 상호작용 시도 시 호출. Holding 진행 시작
	virtual bool ExecuteInteraction(const FInteractionContext &Context) override;

protected:
	/** 상호작용 성공까지 필요한 누르기 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding")
	float RequiredHoldingTime;

	// TODO: Holding 진행 로직에 대한 가상 함수들
	
	/**
	 * [Holding Start] 누르기가 시작될 때 호출됩니다.
	 */
	virtual void OnHoldingStarted(const FInteractionContext &Context);

	/**
	 * [Holding Complete] 지정된 시간을 달성하여 상호작용이 성공했을 때 호출됩니다.
	 */
	virtual void OnHoldingCompleted(const FInteractionContext &Context);

	/**
	 * [Holding Canceled] 도중에 키를 떼거나 조건이 안 맞아 취소되었을 때 호출됩니다.
	 */
	virtual void OnHoldingCanceled(const FInteractionContext &Context);
};
