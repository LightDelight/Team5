// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicInteractionInterface.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Common.generated.h"

/**
 * 집을 수 있는 객체의 공통 로직 모듈 (잡기/놓기/던지기)
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_Interactable_Common
    : public ULogicModuleBase {
  GENERATED_BODY()

public:
  // ICarryLogicInterface 구현
  virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
