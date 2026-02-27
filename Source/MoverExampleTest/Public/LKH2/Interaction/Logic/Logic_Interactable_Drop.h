// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Drop.generated.h"

/**
 * 아이템을 내려놓는(Drop) 기능을 담당하는 로직 모듈입니다.
 * 아이템이 들려있는 상태에서 일반 상호작용(Interact) 시 실행됩니다.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_Interactable_Drop : public ULogicModuleBase {
  GENERATED_BODY()

protected:
  virtual bool PreInteractCheck(const FInteractionContext &Context) override;
  virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
