// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LKH2/Data/ItemStatValue.h"
#include "PresetData.generated.h"

class ULogicModuleBase;

/**
 * 로직 프리셋 — 로직 모듈 구성과 기본 Stats를 정의합니다.
 * ItemData/WorkstationData의 Track 1 (Preset 기반 모듈)으로 사용됩니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UPresetData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** 기본적으로 포함할 로직 모듈 클래스 목록 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset|Modules")
  TArray<TSubclassOf<ULogicModuleBase>> DefaultModuleClasses;

  /** 프리셋의 기본 Stats (Data에 적용 시 복사됨) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset|Stats")
  TMap<FGameplayTag, FItemStatValue> DefaultStats;

  // ─── 필수 태그 미리보기 (읽기 전용) ───

  /** [읽기 전용] 모든 모듈이 요구하는 Stats 태그 총합 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preset|Info",
            meta = (DisplayName = "[필수 태그] 모듈 요구사항"))
  TArray<FGameplayTag> RequiredStatTags;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(
      FPropertyChangedEvent &PropertyChangedEvent) override;
#endif
  virtual void PostLoad() override;

private:
  void RefreshRequiredTags();
};
