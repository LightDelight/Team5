#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/ProcessingLogicModuleBase.h"
#include "Logic_Interactable_Bake.generated.h"

/**
 * 아이템을 굽는 로직을 담당하는 모듈입니다.
 * 일정 시간이 지나면 원본 아이템을 결과물 태그의 아이템으로 변환합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API ULogic_Interactable_Bake : public UProcessingLogicModuleBase
{
	GENERATED_BODY()

public:
	ULogic_Interactable_Bake();

protected:
	/** 자율 진행 시작 조건을 체크합니다. (레시피 존재 여부 확인) */
	virtual bool CanStartProcessing() const override;

	/** 진행 완료 시 실행될 변환 로직 */
	virtual void OnProcessingCompleted_Implementation(const FInteractionContext& Context) override;

protected:
	/** 레시피 조회를 위한 액션 태그 (예: Interaction.Action.Bake) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Bake")
	FGameplayTag BakeActionTag;

	/** 레시피 대조에 실패했을 때 사용할 폴백(Fallback) 결과 아이템 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Bake")
	FGameplayTag FallbackResultItemTag;

	/** 변환이 일어날 슬롯 태그 (기본값: Slot.Main) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing|Bake")
	FGameplayTag TargetSlotTag;
};
