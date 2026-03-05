// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "TimerManager.h"
#include "LogicTaskBase.generated.h"

class ULogicModuleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogicTaskTickDelegate, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogicTaskEventDelegate, class ULogicTaskBase*, TaskInstance);

/**
 * 로직(LogicModuleBase) 실행 도중, 지연 입력이나 특정 시간 경과 등
 * 시간차(Latent) 처리가 필요할 때 사용하는 1회용 비동기 태스크(Async Action) 베이스입니다.
 * (GameplayAbility의 UAbilityTask와 유사한 역할)
 * 
 * 모듈 자체가 상태를 가지지 않고, 이 태스크 객체가 인스턴스화되어 상태를 지닙니다.
 */
UCLASS(Abstract, BlueprintType)
class MOVEREXAMPLETEST_API ULogicTaskBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	ULogicTaskBase(const FObjectInitializer& ObjectInitializer);

	// 태스크 진행 중 매 프레임 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnLogicTaskTickDelegate OnTick;

	// 태스크가 성공적으로 완료되었을 때 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnLogicTaskEventDelegate OnCompleted;

	// 태스크가 도중에 취소/중단되었을 때 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnLogicTaskEventDelegate OnCanceled;

	/** 지정된 로직 모듈을 부모로 하여 새로운 비동기 태스크를 생성합니다. */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task", meta = (DefaultToSelf = "OwningModule", HidePin = "OwningModule", BlueprintInternalUseOnly = "true"))
	static ULogicTaskBase* CreateLogicTask(class ULogicModuleBase* OwningModule, TSubclassOf<ULogicTaskBase> TaskClass, const FInteractionContext& Context);

	/** 태스크의 현재 진행도(0.0 ~ 1.0)를 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task")
	virtual float GetProgressRatio() const;

	/** 태스크 시작 시 저장된 Context를 반환합니다. */
	const FInteractionContext& GetCachedContext() const { return CachedContext; }

	/**
	 * 해당 컨텍스트(데이터)를 기반으로 태스크의 내부 로직을 시작합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task", meta = (BlueprintInternalUseOnly = "true"))
	virtual void ReadyForActivation();

	/** 외부에서 태스크를 강제 종료할 때 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task")
	virtual void EndTask();

	/**
	 * 외부에서 태스크를 취소할 때 호출합니다.
	 * OnCanceled를 먼저 실행한 후 EndTask()를 호출합니다.
	 * EndTask()를 직접 호출하면 OnCanceled가 발식되지 않으므로,
	 * 취소 의도가 있는 코드는 마족 이 함수를 사용해야 합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task")
	virtual void CancelTask();

	/** 태스크 요구 시간. (기본값으로 활용 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Task")
	float RequiredTime = 0.0f;

protected:
	UPROPERTY()
	TWeakObjectPtr<ULogicModuleBase> OwningLogicModule;

	FInteractionContext CachedContext;

	bool bIsTaskActive = false;

	/** 태스크 진행 경과 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Logic|Task")
	float ElapsedTime = 0.0f;

	/** 실제 진행 업데이트 등을 구현할 수 있는 틱 함수 (ReadyForActivation 시 타이머로 자동 호출됨) */
	virtual void TickTask(float DeltaTime);

	virtual void OnTaskEnded();

protected:
	/** 주기적 TickTask 호출을 관리하는 타이머 핸들 */
	FTimerHandle TickTimerHandle;

	/** TickTask 호출 간격 (초) */
	static constexpr float TaskTickInterval = 0.1f;
};
