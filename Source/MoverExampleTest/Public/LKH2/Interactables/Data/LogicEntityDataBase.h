// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "GameplayTagContainer.h"
#include "LogicEntityDataBase.generated.h"

class ULogicModuleBase;
class UPresetData;

/**
 * 아이템과 워크스테이션 데이터의 공통 로직 구조를 정의하는 베이스 클래스.
 */
UCLASS(Abstract, BlueprintType)
class MOVEREXAMPLETEST_API ULogicEntityDataBase : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** [Track 2] 추가 로직 모듈 */
  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Logic|Modules")
  TArray<TObjectPtr<ULogicModuleBase>> AdditionalModules;

  /** 개별 엔티티 고유 스탯 (무게, 작업 속도 등) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stats")
  TMap<FGameplayTag, FItemStatValue> EntityStats;

  /**
   * AdditionalModules 배열을 반환합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Modules")
  TArray<ULogicModuleBase *> GetAllModules() const;

};
