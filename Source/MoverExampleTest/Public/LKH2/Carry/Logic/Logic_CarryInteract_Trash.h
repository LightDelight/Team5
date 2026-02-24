// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "Logic_CarryInteract_Trash.generated.h"

class AActor;
class UCarryComponent;
class AContainerItemBase;

/**
 * 쓰레기통 로직 모듈.
 * - 일반 아이템: ForceDrop 후 파괴 (Destroy)
 * - ContainerItemBase(접시 등): 컨테이너 자체는 유지하고,
 *   블랙보드에 거치된 아이템만 파괴하여 비움
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_CarryInteract_Trash
    : public ULogicModuleBase {
  GENERATED_BODY()

protected:
  virtual bool PerformInteraction(const FCarryContext &Context) override;
};
