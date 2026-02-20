// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CarryableLogicInterface.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCarryableLogicInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 모듈 중 Carry와 관련된 기능을 추가하고 싶을 때 상속하는 인터페이스
 */
class ICarryableLogicInterface {
  GENERATED_BODY()

  // Add interface functions to this class. This is the class that will be
  // inherited to implement this interface.
public:
  // 아이템을 주울 때 실행할 로직
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Carry")
  void OnModulePickedUp(AActor *Carrier, AActor *ItemTarget);

  // 아이템을 놓을 때 실행할 로직
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Carry")
  void OnModuleDropped(AActor *Carrier, AActor *ItemTarget);

  // 아이템을 던질 때 실행할 로직
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic Module|Carry")
  void OnModuleThrown(AActor *Carrier, AActor *ItemTarget,
                      FVector ThrowVelocity);
};
