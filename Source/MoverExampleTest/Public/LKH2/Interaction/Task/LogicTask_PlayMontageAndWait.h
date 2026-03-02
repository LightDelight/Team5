// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicTaskBase.h"
#include "GameplayTagContainer.h"
#include "LogicTask_PlayMontageAndWait.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogicTaskNotifyDelegate, FGameplayTag, EventTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLogicTaskProgressDelegate, ULogicTaskBase*, Task, float, CurrentStep, float, MaxStep);


/**
 * 몽타주를 재생하고, 재생이 끝날 때까지 대기하거나 특정 AnimNotify가 발생할 때 이벤트를 발송하는 LogicTask입니다.
 * 인터랙션(Holding 등) 중에 캐릭터의 애니메이션 상태를 제어하고 싱크를 맞출 때 사용합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API ULogicTask_PlayMontageAndWait : public ULogicTaskBase
{
	GENERATED_BODY()

public:
	ULogicTask_PlayMontageAndWait(const FObjectInitializer& ObjectInitializer);

	/** 이 태스크가 활성화될 때 PropertyComponent에 등록할 행동 태그 (예: Action.Chop) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Task|Action")
	FGameplayTag ActionTag;

	/** 특정 Notify가 발생할 때마다 호출되는 델리게이트 (단계적 진행도 등 처리에 활용) */
	UPROPERTY(BlueprintAssignable)
	FOnLogicTaskNotifyDelegate OnNotifyEvent;

	/** 진행도가 갱신될 때마다 호출되는 델리게이트 (Module이 이를 구독하여 UI/스탯 갱신 가능) */
	UPROPERTY(BlueprintAssignable)
	FOnLogicTaskProgressDelegate OnProgressUpdated;

	/** 최대 달성해야 할 스텝 수 */
	UPROPERTY(BlueprintReadWrite, Category = "Logic|Task|Progress")
	float MaxStep = 1.0f;

	/** 현재 스텝 수 */
	UPROPERTY(BlueprintReadWrite, Category = "Logic|Task|Progress")
	float CurrentStep = 0.0f;

	/** 로직 모듈에서 진행 이벤트(Notify -> Intent)가 들어왔을 때 스텝을 증가시킵니다. */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task")
	virtual void AdvanceProgress();

	/**
	 * 시작 시 Interactor ↔ TargetActor 거리가 이 값을 초과하면 태스크를 자동 취소합니다.
	 * 0 이하면 거리 모니터링 비활성화.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Logic|Task|Distance")
	float MaxInteractionDistance = 0.0f;

	virtual void ReadyForActivation() override;
	virtual void EndTask() override;

private:
	/** 거리 첨크 타이머 */
	FTimerHandle DistanceCheckTimerHandle;

	/** 0.1초마다 Interactor ↔ TargetActor 거리를 확인하여 초과 시 EndTask() 호출 */
	void CheckDistance();
};
