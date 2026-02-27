// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LogicContextComponent.generated.h"

class ULogicEntityDataBase;
class ULogicModuleBase;
struct FItemStatValue;
struct FGameplayTag;

/**
 * 아이템이나 워크스테이션의 개별적인 상태(Stats 등)를 저장하고 
 * 로직 모듈의 생시주기(초기화)를 전담하는 컴포넌트입니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API ULogicContextComponent : public UActorComponent {
  GENERATED_BODY()

public:
  ULogicContextComponent();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  /** 실제 블랙보드 데이터에 접근합니다. */
  FLogicBlackboard *GetBlackboard() { return &Blackboard; }
  const FLogicBlackboard *GetBlackboard() const { return &Blackboard; }

  /** 초기화 여부를 반환합니다. */
  bool IsLogicInitialized() const { return bLogicInitialized; }

  // [Moved] GetLogicModules() 및 InitializeLogic()은 UInteractableComponent로 이관됨.

  /** 
   * 스탯을 찾습니다 (우선순위: 블랙보드 -> 엔티티 데이터 -> 프리셋)
   */
  const FItemStatValue* FindStat(const FGameplayTag& Tag) const;

  /**
   * 블랙보드(런타임)에 스탯을 설정합니다.
   */
  void SetStat(const FGameplayTag& Tag, const FItemStatValue& Value);

  /**
   * 키 태그를 실제 스탯 태그로 해소합니다.
   */
  FGameplayTag ResolveKey(const FGameplayTag& Key) const;

  /**
   * 블랙보드 데이터만 초기화됨을 표시하는 함수.
   */
  void MarkLogicInitialized(ULogicEntityDataBase* InData)
  {
      bLogicInitialized = true;
      EntityData = InData;
  }

protected:
  /** 로직 상태를 관리하고 멀티플레이 복제를 담당하는 블랙보드입니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Logic")
  FLogicBlackboard Blackboard;

  /** 이중 초기화 방지용 플래그 */
  UPROPERTY(Transient)
  bool bLogicInitialized = false;

  /** 데이터 에셋 참조를 캐싱합니다 (스탯 조회용) */
  UPROPERTY(Transient)
  TObjectPtr<ULogicEntityDataBase> EntityData;
};
