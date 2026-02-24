// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LogicInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class ULogicInteractionInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 아이템 기반(Carryable) 및 상호작용 기반(Workstation) 로직을 통합 관리하는
 * 인터페이스입니다.
 */
class MOVEREXAMPLETEST_API ILogicInteractionInterface {
  GENERATED_BODY()

public:
  /**
   * 아이템이나 워크스테이션에서 상호작용이 일어났을 때 실행할 통합 로직.
   * OnModuleInteract에서 OnLogicInteract로 명칭이 변경되었습니다.
   */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Interaction")
  bool OnLogicInteract(const FCarryContext &Context);
};
