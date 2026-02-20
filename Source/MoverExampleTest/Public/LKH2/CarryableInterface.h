// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CarryableInterface.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UCarryableInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 캐리(들기) 가능한 객체가 구현해야 할 인터페이스
 */
class ICarryableInterface {
  GENERATED_BODY()

public:
  // 아이템이 캐리 모드로 전환될 때 호출됩니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void OnPickedUp(AActor *Carrier);

  // 아이템이 맵에 배치되거나 놓여질 때 호출됩니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void OnDropped();

  // 아이템이 던져질 때 호출됩니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void OnThrown(FVector ThrowVelocity);

  // 아웃라인(외곽선) 표시 여부를 설정합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void SetOutlineEnabled(bool bEnabled);
};
