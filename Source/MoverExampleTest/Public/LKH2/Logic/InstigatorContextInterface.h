// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstigatorContextInterface.generated.h"

class UCarryComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInstigatorContextInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 액터(주로 플레이어 캐릭터)가 상속받아 구현함으로써 자신이 소유한 기능
 * 컴포넌트들을 모듈들에게 O(1) 인터페이스 캐스팅으로 빠르고 안전하게 제공하기
 * 위한 컨텍스트 인터페이스입니다.
 */
class MOVEREXAMPLETEST_API IInstigatorContextInterface {
  GENERATED_BODY()

public:
  /** 플레이어가 보유 중인 들기/내려놓기(Carry) 수행 컴포넌트를 반환합니다. */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "Logic|Instigator")
  UCarryComponent *GetCarryComponent() const;
};
