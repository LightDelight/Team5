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

	/** 진행도 UI를 표시할 슬롯 키 (비워두면 TargetSlotTag와 동일하게 동작) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FGameplayTag UISlotTag;

	/** 프로그레스 고정(동결) 시 사용할 현재 수치 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FGameplayTag CurrentStepTag;

	/** 프로그레스 고정(동결) 시 사용할 최대 수치 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FGameplayTag MaxStepTag;
	
	/** 컨테이너임을 식별하는 데 사용하는 태그 (기본값: Type.Item.Container) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FGameplayTag ContainerTypeTag;

	/** 취소 시 UI를 제거하지 않고 유지할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|UI")
	bool bMaintainUIOnCancel = false;

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

	/** 
	 * 슬롯이나 컨텍스트에서 실제 가공 대상 아이템을 찾습니다. 
	 * 가공 대상이 컨테이너라면 내부의 첫 번째 유효 아이템을 반환할 수 있습니다.
	 */
	virtual class AItemBase* GetTargetItemDeep(const FInteractionContext& Context, class UInteractablePropertyComponent* PropComp, FGameplayTag SlotTag) const;
};
