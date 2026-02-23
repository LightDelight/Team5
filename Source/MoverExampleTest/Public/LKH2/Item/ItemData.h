// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LKH2/Data/ItemStatValue.h"
#include "ItemData.generated.h"

class ULogicModuleBase;
class UPresetData;

/**
 * 아이템 데이터를 정의하는 클래스.
 * 투 트랙 구조:
 *   Track 1 — Preset 기반 모듈 (TSubclassOf, Preset 변경 시 자동 반영)
 *   Track 2 — AdditionalModules (Instanced, 에디터에서 직접 편집 가능)
 */
UCLASS(BlueprintType)
class UItemData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  FText ItemName;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  TObjectPtr<UStaticMesh> ItemMesh;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  float ItemWeight = 10.0f;

  /** Sphere 콜리전 반지름 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Collision")
  float SphereRadius = 32.0f;

  /** 루트(SphereCollision) 기준 메쉬의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Collision")
  FVector MeshRelativeLocation = FVector::ZeroVector;

  // ─── 투 트랙 모듈 시스템 ───

  /** [Track 1] 프리셋 기반 모듈 구성 (변경 시 자동 반영) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Preset")
  TObjectPtr<UPresetData> Preset;

  /** [Track 2] 추가 로직 모듈 (에디터에서 개별 편집 가능) */
  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Item|Modules")
  TArray<TObjectPtr<ULogicModuleBase>> AdditionalModules;

  /** GameplayTag 기반 아이템 설정값 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stats")
  TMap<FGameplayTag, FItemStatValue> ItemStats;

  // ─── 헬퍼 ───

  /**
   * Preset 모듈 + AdditionalModules를 합산한 전체 모듈 배열을 반환합니다.
   * Preset의 TSubclassOf 목록은 CDO(Class Default Object)로 반환됩니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Item|Modules")
  TArray<ULogicModuleBase *> GetAllModules() const;
};
