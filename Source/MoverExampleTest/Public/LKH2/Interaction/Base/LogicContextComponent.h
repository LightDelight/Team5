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
 * 전담하는 컴포넌트입니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API ULogicContextComponent : public UActorComponent {
  GENERATED_BODY()

public:
  ULogicContextComponent();

  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  /** Stats FastArray에 직접 접근합니다. */
  FLogicBlackboardStatSerializer& GetStats() { return Stats; }
  const FLogicBlackboardStatSerializer& GetStats() const { return Stats; }

  /** ObjectBlackboard FastArray에 직접 접근합니다. */
  FLogicBlackboardObjectSerializer& GetObjectBlackboard() { return ObjectBlackboard; }
  const FLogicBlackboardObjectSerializer& GetObjectBlackboard() const { return ObjectBlackboard; }

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
   * 블랙보드(런타임)에서 특정 스탯을 제거합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Blackboard")
  void RemoveStat(FGameplayTag Tag);

  /**
   * 블랙보드(런타임)의 모든 스탯을 제거합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Blackboard")
  void ClearAllStats();

  /**
   * 블랙보드(런타임)에서 특정 오브젝트를 제거합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Blackboard")
  void RemoveObject(FGameplayTag Tag);

  /**
   * 블랙보드(런타임)의 모든 오브젝트를 제거합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Blackboard")
  void ClearAllObjects();

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

  /**
   * 태그를 키값으로 특정 로직 태스크를 저장합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Task")
  void SetTask(FGameplayTag TaskTag, class ULogicTaskBase* NewTask);

  /**
   * 태그로 보관 중인 로직 태스크를 반환합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Task")
  class ULogicTaskBase* GetTask(FGameplayTag TaskTag) const;

  /**
   * 태그로 보관 중인 로직 태스크를 초기화(해제)합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Task")
  void ClearTask(FGameplayTag TaskTag);

protected:

  /** 런타임 수치 Stat을 저장합니다. FastArray 기반으로 클라이언트에 직접 복제됩니다. */
  UPROPERTY(Replicated)
  FLogicBlackboardStatSerializer Stats;

  /** 런타임 오브젝트 래퍼런스를 저장합니다. FastArray 기반으로 클라이언트에 직접 복제됩니다. */
  UPROPERTY(Replicated)
  FLogicBlackboardObjectSerializer ObjectBlackboard;

  /** 이중 초기화 방지용 플래그 */
  UPROPERTY(Transient)
  bool bLogicInitialized = false;

  /** 데이터 에셋 참조를 캐싱합니다 (스탯 조회용) */
  UPROPERTY(Transient)
  TObjectPtr<ULogicEntityDataBase> EntityData;

  /** 현재 진행 중인 로직 태스크들 (진행 및 취소 트래킹용) */
  UPROPERTY(Transient)
  TMap<FGameplayTag, TObjectPtr<class ULogicTaskBase>> ActiveTasks;
};
