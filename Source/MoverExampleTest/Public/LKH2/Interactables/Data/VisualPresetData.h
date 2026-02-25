// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/WorkStation/WorkStationBase.h"
#include "VisualPresetData.generated.h"

class UStaticMesh;

/**
 * 비주얼 프리셋 대상 타입
 */
UENUM(BlueprintType)
enum class EVisualPresetType : uint8 {
  Item UMETA(DisplayName = "Item"),
  Workstation UMETA(DisplayName = "Workstation"),
};

/**
 * 비주얼 프리셋 — 스폰 클래스, 메쉬, 콜리전/오프셋 등 시각적 기본값을 정의합니다.
 * ItemData/WorkstationData에서 bUseCustomVisuals가 false일 때 이 값을 사용합니다.
 *
 * PresetType에 따라 에디터에서 해당 타입의 필드만 표시됩니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UVisualPresetData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** 이 비주얼 프리셋의 대상 타입 (Item 또는 Workstation) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VisualPreset")
  EVisualPresetType PresetType = EVisualPresetType::Item;

  // ─── Item 비주얼 (PresetType == Item일 때만 표시) ───

  /** 아이템 스폰 시 사용할 Actor 클래스 기본값 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Item",
            meta = (EditCondition = "PresetType == EVisualPresetType::Item",
                    EditConditionHides))
  TSubclassOf<AItemBase> DefaultItemClass;

  /** 아이템 메쉬 기본값 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Item",
            meta = (EditCondition = "PresetType == EVisualPresetType::Item",
                    EditConditionHides))
  TObjectPtr<UStaticMesh> DefaultItemMesh;

  /** Sphere 콜리전 반지름 기본값 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Item",
            meta = (EditCondition = "PresetType == EVisualPresetType::Item",
                    EditConditionHides))
  float DefaultSphereRadius = 32.0f;

  /** 루트(SphereCollision) 기준 메쉬의 상대 위치 오프셋 기본값 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Item",
            meta = (EditCondition = "PresetType == EVisualPresetType::Item",
                    EditConditionHides))
  FVector DefaultMeshRelativeLocation = FVector::ZeroVector;

  // ─── Workstation 비주얼 (PresetType == Workstation일 때만 표시) ───

  /** 워크스테이션 스폰 시 사용할 Actor 클래스 기본값 */
  UPROPERTY(
      EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Workstation",
      meta = (EditCondition = "PresetType == EVisualPresetType::Workstation",
              EditConditionHides))
  TSubclassOf<AWorkStationBase> DefaultWorkstationClass;

  /** 워크스테이션 메쉬 기본값 */
  UPROPERTY(
      EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Workstation",
      meta = (EditCondition = "PresetType == EVisualPresetType::Workstation",
              EditConditionHides))
  TObjectPtr<UStaticMesh> DefaultWorkstationMesh;

  /** Box 콜리전 크기 (Half-Extent) 기본값 */
  UPROPERTY(
      EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Workstation",
      meta = (EditCondition = "PresetType == EVisualPresetType::Workstation",
              EditConditionHides))
  FVector DefaultBoxExtent = FVector(50.0f, 50.0f, 50.0f);

  /** 루트(RootMesh) 기준 BoxCollision의 상대 위치 오프셋 기본값 */
  UPROPERTY(
      EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Workstation",
      meta = (EditCondition = "PresetType == EVisualPresetType::Workstation",
              EditConditionHides))
  FVector DefaultBoxRelativeLocation = FVector::ZeroVector;

  /** 루트(RootMesh) 기준 CarryInteractComponent의 상대 위치 오프셋 기본값 */
  UPROPERTY(
      EditAnywhere, BlueprintReadOnly, Category = "VisualPreset|Workstation",
      meta = (EditCondition = "PresetType == EVisualPresetType::Workstation",
              EditConditionHides))
  FVector DefaultInteractRelativeLocation = FVector::ZeroVector;
};
