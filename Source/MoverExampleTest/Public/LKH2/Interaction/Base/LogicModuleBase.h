// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Base/LogicInteractionInterface.h"
#include "LogicModuleBase.generated.h"

class AActor;

/**
 * 기본 데이터를 가지고 있거나, 로직을 담을 수 있는 빈 껍데기 모듈.
 * 파생 모듈들은 InitializeLogic / OnConstructionLogic 을 오버라이드하여
 * 소유 액터 컨텍스트에서 초기화 및 에디터 미리보기를 수행합니다.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API ULogicModuleBase : public UObject,
                                             public ILogicInteractionInterface {
  GENERATED_BODY()

public:
  /** ILogicInteractionInterface 구현: ExecuteInteraction과 시스템 통합 */
  virtual bool OnLogicInteract_Implementation(const FInteractionContext &Context) override;

  /**
   * 상호작용 실행을 위한 템플릿 메서드.
   * 사전 검사 -> 핵심 실행 -> 사후 처리 과정을 캡슐화합니다.
   */
  virtual bool ExecuteInteraction(const FInteractionContext &Context);

protected:
  /**
   * [Pre-Logic] 상호작용 실행 전 유효성 및 조건을 검사합니다.
   * @return true이면 핵심 로직(PerformInteraction)을 실행합니다.
   */
  virtual bool PreInteractCheck(const FInteractionContext &Context) { return true; }

  /**
   * [Core-Logic] 실제 상호작용 행동을 수행합니다.
   */
  virtual bool PerformInteraction(const FInteractionContext &Context) { return false; }

  /**
   * [Post-Logic] 상호작용 실행 후(성공/실패 무관) 정리 및 부가 작업을 수행합니다.
   */
  virtual void PostInteract(const FInteractionContext &Context, bool bExecutionResult) {}
  
  /** 이 모듈을 소유한 액터를 반환합니다. */
  UFUNCTION(BlueprintCallable, Category = "Logic")
  AActor* GetOwner() const { return OwnerActor.Get(); }

protected:
  UPROPERTY(BlueprintReadOnly, Category = "Logic")
  TObjectPtr<AActor> OwnerActor;

public:
  /**
   * 소유 액터의 BeginPlay에서 호출됩니다.
   * 월드/액터 컨텍스트가 필요한 런타임 초기화를 여기서 수행합니다.
   */
  virtual void InitializeLogic(AActor *InOwnerActor) { OwnerActor = InOwnerActor; }

  /**
   * 소유 액터의 OnConstruction에서 호출됩니다.
   * 에디터에서 즉시 시각적 결과를 확인할 수 있는 로직을 여기서 수행합니다.
   */
  virtual void OnConstructionLogic(AActor *InOwnerActor) { OwnerActor = InOwnerActor; }

  /**
   * 이 모듈이 정상 작동하기 위해 DA의 ItemStats에 설정되어야 하는
   * GameplayTag 목록을 반환합니다. 기획자에게 필수 태그를 안내하는 용도입니다.
   */
  virtual TArray<FGameplayTag> GetRequiredStatTags() const {
    return TArray<FGameplayTag>();
  }
};
