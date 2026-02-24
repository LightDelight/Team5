// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LKH2/Data/ItemStatValue.h"
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
  /** [Track 1] 로직 프리셋 (모듈 + Stats) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic|Preset")
  TObjectPtr<UPresetData> Preset;

  /** [Track 2] 추가 로직 모듈 */
  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Logic|Modules")
  TArray<TObjectPtr<ULogicModuleBase>> AdditionalModules;

  /** 개별 엔티티 고유 스탯 (무게, 작업 속도 등) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stats")
  TMap<FGameplayTag, FItemStatValue> EntityStats;

  /**
   * Preset 모듈 + AdditionalModules를 합산한 전체 모듈 배열을 반환합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Logic|Modules")
  TArray<ULogicModuleBase *> GetAllModules() const;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(
      FPropertyChangedEvent &PropertyChangedEvent) override;
#endif
  virtual void PostLoad() override;

protected:
  /** 하위 클래스에서 미리보기 필드를 갱신하도록 구현합니다. */
  virtual void RefreshPresetPreview() {}
};
