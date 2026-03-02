#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "ProcessingLogicModuleBase.generated.h"

/**
 * 아이템이 거치되었을 때 자동으로 시작하여 일정 시간 후 결과를 내는 
 * 자율 진행형 로직 모듈의 베이스 클래스입니다. (예: 조리, 가공 등)
 */
UCLASS(Abstract, Blueprintable)
class MOVEREXAMPLETEST_API UProcessingLogicModuleBase : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	UProcessingLogicModuleBase();

	/** 상호작용 실행 (Intent 분기 처리) */
	virtual bool ExecuteInteraction(const FInteractionContext& Context) override;

protected:
	/** 진행에 필요한 총 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing")
	float ProcessingDuration = 5.0f;

	/** 시작으로 판정할 의도 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Intent")
	FGameplayTag StartIntentTag;

	/** 중단으로 판정할 의도 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Intent")
	FGameplayTag StopIntentTag;

	/** 진행 상태 UI에 사용할 시작 시간 스탯 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|UI")
	FGameplayTag StartTimeTag;

	/** 진행 상태 UI에 사용할 종료 시간 스탯 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|UI")
	FGameplayTag EndTimeTag;

	/** 진행에 필요한 최소 아이템 개수 (기본 1개) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Condition")
	int32 RequiredItemCount = 1;

	/** 이 프로세싱 태스크를 식별하고 컨텍스트에 저장할 때 사용할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Task")
	FGameplayTag ProcessingTaskTag;

protected:
	/** 자율 진행 시작 조건을 체크합니다. */
	virtual bool CanStartProcessing() const;

	/** 진행을 시작합니다. (UI 활성화 및 태스크 생성) */
	virtual void StartProcessing(const FInteractionContext& Context);

	/** 진행을 명시적으로 중단합니다. (UI 해제 및 태스크 취소) */
	virtual void StopProcessing(const FInteractionContext& Context);

	/** 진행 완료 시 실행될 핵심 보상/변환 로직입니다. (BP에서 오버라이드 가능) */
	UFUNCTION(BlueprintNativeEvent, Category = "Processing")
	void OnProcessingCompleted(const FInteractionContext& Context);

	/** 상호작용 성공 시 처리 (LogicTask_Wait 완료 시 호출됨) */
	virtual void HandleTaskCompleted(class ULogicTaskBase* TaskInstance) override;

	/** 상호작용 취소 시 처리 */
	virtual void HandleTaskCanceled(class ULogicTaskBase* TaskInstance) override;
};
