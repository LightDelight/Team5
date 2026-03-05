#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicTaskBase.h"
#include "LogicTask_Wait.generated.h"

/**
 * 단순히 지정된 시간만큼 대기하는 Latent 태스크입니다.
 * 타임스탬프 기반의 자율 진행 로직에서 핵심적인 시간 경과 처리를 담당합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API ULogicTask_Wait : public ULogicTaskBase
{
	GENERATED_BODY()

public:
	ULogicTask_Wait(const FObjectInitializer& ObjectInitializer);

	/** 
	 * 지정된 시간(WaitTime)만큼 대기하는 태스크를 생성합니다. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Logic|Task", meta = (DefaultToSelf = "OwningModule", HidePin = "OwningModule", BlueprintInternalUseOnly = "true"))
	static ULogicTask_Wait* Wait(class ULogicModuleBase* OwningModule, float WaitTime, const FInteractionContext& Context, UObject* InOuter = nullptr);

	virtual void ReadyForActivation() override;

protected:
	virtual void TickTask(float DeltaTime) override;
};
