// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_PickUp.generated.h"

/**
 * 아이템을 줍는(PickUp) 기능을 담당하는 로직 모듈입니다.
 * 아이템이 들려있지 않은 상태에서만 실행됩니다.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_Interactable_PickUp : public ULogicModuleBase {
  GENERATED_BODY()

protected:
  virtual bool PreInteractCheck(const FInteractionContext &Context) override;
  virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
