#pragma once

#include "LKH2/Interaction/Holding/HoldingStepLogicModuleBase.h"
#include "Logic_Interactable_Holding_Chop.generated.h"

/**
 * 도마에 올라간 물건을 썰기(Chop) 전용으로 특화된 로직 모듈입니다.
 * 현재는 모든 핵심 기능(Montage + Notify + Recipe)이 UHoldingStepLogicModuleBase로 이관되었습니다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API ULogic_Interactable_Holding_Chop : public UHoldingStepLogicModuleBase
{
	GENERATED_BODY()

public:
	ULogic_Interactable_Holding_Chop();

protected:
	virtual void OnHoldingCompleted_Implementation(const FInteractionContext& Context) override;
};
