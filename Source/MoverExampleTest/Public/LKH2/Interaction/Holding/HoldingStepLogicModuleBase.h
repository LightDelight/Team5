// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "HoldingStepLogicModuleBase.generated.h"

/**
 * 일정 시간 누르고 있어야 하는 지속성 상호작용 로직의 베이스 모듈입니다.
 * 
 * Holding(누르고 있기) 상호작용은 시작(Start), 진행(Tick), 성공(Complete), 취소(Cancel) 로 나뉠 수 있습니다.
 * 
 * TODO: 추후 Manager 쪽과 연계하여 Tick 갱신 및 UI 연동 등을 확장할 수 있습니다.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API UHoldingStepLogicModuleBase : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 생성자
	UHoldingStepLogicModuleBase();

	/** 태스크를 식별하고 Context에 보관할 때 사용하는 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag TaskTag;

	/** 캐릭터의 몽타주를 재생하기 위한 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag ActionTag;

	/** 레시피 조회를 위한 태그 (예: Interaction.Chop) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag RecipeTag;

	/** 애니메이션 Notify 등 진행도를 올릴 때 수신할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag ProgressIntentTag;

	/** 아이템 블랙보드에 진행도를 기록/복구할 때 사용할 스탯 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag ProgressStatTag;

	/** 최대 진행도를 기록/표시할 때 사용할 스탯 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag MaxProgressStatTag;

	/** 상호작용 도중 키를 떼거나 취소할 때 수신할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag CancelIntentTag;

	/** 친인 행동을 순서에맞게 실행하기 위한 접근 방식으로 헸퍼를 물리적으로 화면에서 제거하기 위한 피첨 형식 */
	/** 아이템이 위치한 슬롯 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Tag")
	FGameplayTag SlotTag;

	/** 총 진행 단계 (예: 5번 썰어야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Value")
	float MaxStep = 5.0f;

	/** 홈딩 중 이 거리를 초과하면 자동 취소 (cm, 0 이하면 비활성화) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Holding|Value")
	float MaxInteractionDistance = 200.0f;

	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;

	/** 태스크가 성공적으로 완료되었을 때 호출되는 핸들러입니다. */
	virtual void HandleTaskCompleted(class ULogicTaskBase* TaskInstance) override;

	/** 태스크가 도중에 취소되었을 때 호출되는 핸들러입니다. */
	virtual void HandleTaskCanceled(class ULogicTaskBase* TaskInstance) override;

	/** 태스크로부터 진행도 갱신 신호를 받았을 때 UI 및 스탯 처리를 수행합니다. */
	virtual void HandleTaskProgressUpdated(class ULogicTaskBase* TaskInstance, float InCurrentStep, float InMaxStep) override;

protected:
	/** 사용할 비동기 태스크 클래스 (C++ 내부에서 결정됨) */
	UPROPERTY()
	TSubclassOf<class ULogicTaskBase> TaskClass;


	/** [Holding Start] 누르기가 시작될 때 호출됩니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Logic|Holding|Events")
	void OnHoldingStarted(const FInteractionContext &Context);
	virtual void OnHoldingStarted_Implementation(const FInteractionContext &Context);

	/** [Holding Complete] 지정된 시간을 달성하여 상호작용이 성공했을 때 호출됩니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Logic|Holding|Events")
	void OnHoldingCompleted(const FInteractionContext &Context);
	virtual void OnHoldingCompleted_Implementation(const FInteractionContext &Context);

	/** [Holding Canceled] 도중에 키를 떼거나 조건이 안 맞아 취소되었을 때 호출되었을 때 호출됩니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "Logic|Holding|Events")
	void OnHoldingCanceled(const FInteractionContext &Context);
	virtual void OnHoldingCanceled_Implementation(const FInteractionContext &Context);

	/**
	 * 태스크를 생성한 후, 구체적인 속성(MaxStep, ActionTag 등)을 주입합니다.
	 * 파생 클래스에서 오버라이드하여 커스텀 태스크 설정을 수행할 수 있습니다.
	 */
	virtual void ConfigureTask(class ULogicTaskBase* Task, const FInteractionContext &Context);

	/** Context의 TargetActor로부터 LogicContextComponent를 안전하게 조회하는 헬퍼 */
	class ULogicContextComponent* GetContextComponent(const FInteractionContext& Context) const;
};
