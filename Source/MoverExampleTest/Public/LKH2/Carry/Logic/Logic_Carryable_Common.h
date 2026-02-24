// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "Logic_Carryable_Common.generated.h"

/**
 * 집을 수 있는 객체의 공통 로직 모듈 (잡기/놓기/던지기)
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_Carryable_Common
    : public ULogicModuleBase {
  GENERATED_BODY()

public:
  // ICarryLogicInterface 구현
  virtual bool PerformInteraction(const FCarryContext &Context) override;
};
