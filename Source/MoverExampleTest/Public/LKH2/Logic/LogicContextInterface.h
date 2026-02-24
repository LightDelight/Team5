// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LogicContextInterface.generated.h"

class UCarryableComponent;
class UCarryInteractComponent;
struct FLogicBlackboard;
struct FItemStatValue;
struct FGameplayTag;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class ULogicContextInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * 액터가 상속받아 구현함으로써 자신에게 부착된 다양한 로직/컴포넌트 객체의
 * 참조를 중재자(Mediator) 역할로 외부 모듈에 전달하기 위한 인터페이스입니다.
 */
class MOVEREXAMPLETEST_API ILogicContextInterface {
  GENERATED_BODY()

public:
  /** 아이템이 들려질 수 있는 상태 등을 관리하는 컴포넌트를 반환합니다. */
  virtual UCarryableComponent *GetCarryableComponent() const = 0;

  /** 워크스테이션 등이 아이템 거치 스냅 포인트를 관리하는 컴포넌트를
   * 반환합니다. */
  virtual UCarryInteractComponent *GetCarryInteractComponent() const = 0;

  /** 실제 런타임 상태 데이터를 보관하는 블랙보드 포인터를 반환합니다. */
  virtual FLogicBlackboard *GetLogicBlackboard() = 0;

  /**
   * 이 액터를 정의하는 DataAsset의 ItemStats 맵에서
   * 주어진 태그에 해당하는 값을 반환합니다.
   * 찾지 못하면 nullptr을 반환합니다.
   */
  virtual const FItemStatValue *FindStat(const FGameplayTag &Tag) const = 0;
};
