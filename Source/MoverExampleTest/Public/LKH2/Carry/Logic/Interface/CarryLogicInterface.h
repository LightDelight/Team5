// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "UObject/Interface.h"
#include "CarryLogicInterface.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCarryLogicInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 아이템 기반(Carryable) 및 상호작용 기반(Workstation) 로직을 통합 관리하는
 * 인터페이스입니다. 분리되었던 PickedUp, Dropped, Interact 메시지를 하나로 묶고
 * 상태 폴링으로 동작을 결정합니다.
 */
class MOVEREXAMPLETEST_API ICarryLogicInterface {
  GENERATED_BODY()

public:
  // 아이템이나 워크스테이션에서 상호작용이 일어났을 때 실행할 통합 로직
  // 내부에서는 플레이어나 대상의 StateComponent 등을 확인하여 줍기/놓기/투척
  // 등의 분기를 처리합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Carry")
  bool OnModuleInteract(
      AActor *Interactor, AActor *TargetActor,
      ECarryInteractionType InteractionType = ECarryInteractionType::Interact);
};
