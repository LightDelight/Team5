// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Data/LogicEntityDataBase.h"
#include "GameplayTagContainer.h"
#include "LKH2/Data/ItemStatValue.h"
#include "ItemData.generated.h"

class ULogicModuleBase;
class UPresetData;
class UVisualPresetData;
class UStaticMesh;
class AItemBase;

/**
 * 아이템 데이터를 정의하는 클래스.
 * 투 트랙 구조:
 *   Track 1 — Preset 기반 (로직: PresetData, 비주얼: VisualPresetData)
 *   Track 2 — Custom (AdditionalModules, bUseCustomVisuals → Custom* 필드)
 * 무게(Weight)는 Stats 시스템 (ItemStats / DefaultStats)으로 관리합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UItemData : public ULogicEntityDataBase {
  GENERATED_BODY()

public:
  /** 아이템 식별을 위한 상위 수준의 고유 태그 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identification")
  FGameplayTag ItemTag;

  // ─── 투 트랙: 로직 (부모 클래스에서 상속받음: EntityStats) ───

  // ─── Preset 미리보기 (읽기 전용) ───

  /** [읽기 전용] Preset에서 가져온 모듈 클래스 목록 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Preset",
            meta = (DisplayName = "[Preset] 모듈"))
  TArray<TSubclassOf<ULogicModuleBase>> PresetModules;

  /** [읽기 전용] Preset에서 가져온 기본 Stats */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Preset",
            meta = (DisplayName = "[Preset] 기본 Stats"))
  TMap<FGameplayTag, FItemStatValue> ItemPresetStats;

  /** [읽기 전용] 모듈들이 요구하는 필수 Stats 태그 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Preset",
            meta = (DisplayName = "[필수 태그] 모듈 요구사항"))
  TArray<FGameplayTag> ItemRequiredTags;

  // ─── 투 트랙: 비주얼 ───

  /** [Track 1] 비주얼 프리셋 (이름, 메쉬, 콜리전/오프셋 기본값) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
  TObjectPtr<UVisualPresetData> VisualPreset;

  /** [Track 2] 체크하면 아래 커스텀 비주얼 값을 사용, 해제하면 VisualPreset 사용
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
  bool bUseCustomVisuals = false;

  /** [커스텀] 아이템 스폰 클래스 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  TSubclassOf<AItemBase> CustomItemClass;

  /** [커스텀] 아이템 메쉬 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  TObjectPtr<UStaticMesh> CustomItemMesh;

  /** [커스텀] Sphere 콜리전 반지름 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  float CustomSphereRadius = 32.0f;

  /** [커스텀] 루트(SphereCollision) 기준 메쉬의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  FVector CustomMeshRelativeLocation = FVector::ZeroVector;

  // ─── 헬퍼 ───

  /** 유효 스폰 클래스 반환 (Custom → VisualPreset → nullptr) */
  UFUNCTION(BlueprintCallable, Category = "Item|Visual")
  TSubclassOf<AItemBase> GetEffectiveItemClass() const;

  /** 유효 메쉬 반환 (Custom → VisualPreset → nullptr) */
  UFUNCTION(BlueprintCallable, Category = "Item|Visual")
  UStaticMesh *GetEffectiveItemMesh() const;

  /** 유효 Sphere 반지름 반환 */
  UFUNCTION(BlueprintCallable, Category = "Item|Visual")
  float GetEffectiveSphereRadius() const;

  /** 유효 메쉬 오프셋 반환 */
  UFUNCTION(BlueprintCallable, Category = "Item|Visual")
  FVector GetEffectiveMeshRelativeLocation() const;

  /** Stats에서 무게를 조회 (Tag: Item.Weight, 기본값 10.0f) */
  UFUNCTION(BlueprintCallable, Category = "Item|Stats")
  float GetEffectiveWeight() const;

  /**
   * 전체 모듈 배열 반환 (부모 클래스 메서드 재정의 X, 부모 기능 사용)
   */

protected:
  /** Preset의 모듈/Stats를 미리보기 필드에 복사 */
  virtual void RefreshPresetPreview() override;
};
