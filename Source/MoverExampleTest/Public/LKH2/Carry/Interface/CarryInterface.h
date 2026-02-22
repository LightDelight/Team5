// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CarryInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCarryInterface : public UInterface {
  GENERATED_BODY()
};

UENUM(BlueprintType)
enum class ECarryInteractionType : uint8 {
  Interact UMETA(DisplayName = "Interact (Pick Up or Drop)"),
  Throw UMETA(DisplayName = "Throw")
};

/**
 * 캐리(들기) 가능한 객체가 구현해야 할 인터페이스
 */
class ICarryInterface {
  GENERATED_BODY()

public:
  // 누군가가 이 액터에 대해 들기/놓기/투척 등의 캐리 상호작용을 시도했을 때
  // 호출됩니다. 내부에서 전달된 Type과 현재 상태(ItemState)를 확인하여
  // 적절한 동작(로직 모듈로의 위임)을 수행합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  bool OnCarryInteract(
      AActor *Interactor,
      ECarryInteractionType InteractionType = ECarryInteractionType::Interact);

  // 아웃라인(외곽선) 표시 여부를 설정합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void SetOutlineEnabled(bool bEnabled);
};
