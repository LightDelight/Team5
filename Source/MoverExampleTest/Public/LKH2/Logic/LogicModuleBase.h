// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "LogicModuleBase.generated.h"

class AActor;

/**
 * 기본 데이터를 가지고 있거나, 로직을 담을 수 있는 빈 껍데기 모듈.
 * 파생 모듈들은 InitializeLogic / OnConstructionLogic 을 오버라이드하여
 * 소유 액터 컨텍스트에서 초기화 및 에디터 미리보기를 수행합니다.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API ULogicModuleBase : public UObject {
  GENERATED_BODY()

public:
  /**
   * 소유 액터의 BeginPlay에서 호출됩니다.
   * 월드/액터 컨텍스트가 필요한 런타임 초기화를 여기서 수행합니다.
   */
  virtual void InitializeLogic(AActor *OwnerActor) {}

  /**
   * 소유 액터의 OnConstruction에서 호출됩니다.
   * 에디터에서 즉시 시각적 결과를 확인할 수 있는 로직을 여기서 수행합니다.
   */
  virtual void OnConstructionLogic(AActor *OwnerActor) {}

  /**
   * 이 모듈이 정상 작동하기 위해 DA의 ItemStats에 설정되어야 하는
   * GameplayTag 목록을 반환합니다. 기획자에게 필수 태그를 안내하는 용도입니다.
   */
  virtual TArray<FGameplayTag> GetRequiredStatTags() const {
    return TArray<FGameplayTag>();
  }
};
