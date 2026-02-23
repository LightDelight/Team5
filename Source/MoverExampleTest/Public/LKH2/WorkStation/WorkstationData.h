// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LKH2/Data/ItemStatValue.h"
#include "WorkstationData.generated.h"

class ULogicModuleBase;
class UPresetData;

/**
 * 워크스테이션(작업대) 데이터를 정의하는 클래스.
 * 투 트랙 구조:
 *   Track 1 — Preset 기반 모듈 (TSubclassOf, Preset 변경 시 자동 반영)
 *   Track 2 — AdditionalModules (Instanced, 에디터에서 직접 편집 가능)
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UWorkstationData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation")
  FText WorkstationName;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation")
  TObjectPtr<UStaticMesh> WorkstationMesh;

  /** Box 콜리전 크기 (Half-Extent) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Collision")
  FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);

  /** 루트(RootMesh) 기준 BoxCollision의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Collision")
  FVector BoxRelativeLocation = FVector::ZeroVector;

  // ─── 투 트랙 모듈 시스템 ───

  /** [Track 1] 프리셋 기반 모듈 구성 (변경 시 자동 반영) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Preset")
  TObjectPtr<UPresetData> Preset;

  /** [Track 2] 추가 로직 모듈 (에디터에서 개별 편집 가능) */
  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Workstation|Modules")
  TArray<TObjectPtr<ULogicModuleBase>> AdditionalModules;

  /** GameplayTag 기반 워크스테이션 설정값 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workstation|Stats")
  TMap<FGameplayTag, FItemStatValue> ItemStats;

  // ─── 헬퍼 ───

  /**
   * Preset 모듈 + AdditionalModules를 합산한 전체 모듈 배열을 반환합니다.
   * Preset의 TSubclassOf 목록은 CDO(Class Default Object)로 반환됩니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Modules")
  TArray<ULogicModuleBase *> GetAllModules() const;
};
