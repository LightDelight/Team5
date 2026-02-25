// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "LogicContextInterface.generated.h"

class UInteractableComponent;
struct FLogicBlackboard;
struct FItemStatValue;

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
  /** 아이템이나 작업대의 상호작용 관련 컴포넌트를 반환합니다. */
  virtual UInteractableComponent *GetInteractableComponent() const = 0;

  /** 실제 런타임 상태 데이터를 보관하는 블랙보드 포인터를 반환합니다. */
  virtual FLogicBlackboard *GetLogicBlackboard() = 0;

  /**
   * 이 액터를 정의하는 DataAsset의 ItemStats 맵에서
   * 주어진 태그에 해당하는 값을 반환합니다.
   * (Blackboard에 런타임 값이 있으면 우선 반환합니다.)
   * 찾지 못하면 nullptr을 반환합니다.
   */
  virtual const FItemStatValue *FindStat(const FGameplayTag &Tag) const = 0;

  /**
   * 이 액터의 블랙보드(런타임 상태)에 Stat 값을 설정합니다.
   * 성공 시 네트워크를 통해 클라이언트로 복제됩니다.
   */
  virtual void SetStat(const FGameplayTag &Tag,
                       const FItemStatValue &Value) = 0;

  /**
   * 주어진 키에 대해 Stats 시스템에 등록된 오버라이드 태그가 있는지 확인합니다.
   * @param Key 기본 키 태그 (예: 로직 모듈의 StoredItemKey 필드 값)
   * @return Stats에 해당 키의 Tag 타입 값이 있으면 그 값을, 없으면 입력받은 Key를 반환합니다.
   */
  virtual FGameplayTag ResolveKey(const FGameplayTag &Key) const = 0;

  /** 이 액터가 보유한 로직 모듈 목록을 반환합니다. */
  virtual TArray<class ULogicModuleBase *> GetLogicModules() const = 0;
};
