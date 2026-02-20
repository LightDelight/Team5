// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CarryInteractLogicInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCarryInteractLogicInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 상호작용 관련 로직 모듈을 구현할 때 상속하는 인터페이스 (예: Workstation)
 */
class MOVEREXAMPLETEST_API ICarryInteractLogicInterface {
  GENERATED_BODY()

public:
  // 워크스테이션 같은 곳에 인터랙션 할 때 실행할 로직 (아이템 올려놓기/회수 등)
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Interact")
  void OnModuleInteract(AActor *Interactor, AActor *TargetWorkstation);
};
