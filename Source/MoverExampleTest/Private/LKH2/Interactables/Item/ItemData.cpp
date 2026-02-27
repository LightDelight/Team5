// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interactables/Data/VisualPresetData.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"

// GetAllModules is now handled by ULogicEntityDataBase base class

// ─── 비주얼 헬퍼 ───

TSubclassOf<AItemBase> UItemData::GetEffectiveItemClass() const {
  if (bUseCustomVisuals && CustomItemClass) {
    return CustomItemClass;
  }
  return VisualPreset ? VisualPreset->DefaultItemClass : nullptr;
}

UStaticMesh *UItemData::GetEffectiveItemMesh() const {
  if (bUseCustomVisuals && CustomItemMesh) {
    return CustomItemMesh;
  }
  return VisualPreset ? VisualPreset->DefaultItemMesh.Get() : nullptr;
}

float UItemData::GetEffectiveSphereRadius() const {
  if (bUseCustomVisuals) {
    return CustomSphereRadius;
  }
  return VisualPreset ? VisualPreset->DefaultSphereRadius : 32.0f;
}

FVector UItemData::GetEffectiveMeshRelativeLocation() const {
  if (bUseCustomVisuals) {
    return CustomMeshRelativeLocation;
  }
  return VisualPreset ? VisualPreset->DefaultMeshRelativeLocation
                      : FVector::ZeroVector;
}

// ─── Stats 헬퍼 ───

float UItemData::GetEffectiveWeight() const {
  // 우선도: ItemStats → Preset DefaultStats → 기본값 10.0f
  static const FGameplayTag WeightTag =
      FGameplayTag::RequestGameplayTag(FName(TEXT("Item.Weight")));

  // 1. 자체 Stats에서 찾기
  if (const FItemStatValue *Found = EntityStats.Find(WeightTag)) {
    if (Found->Type == EItemStatType::Float) {
      return Found->FloatValue;
    }
  }

  // 2. 하드코딩 기본값
  return 10.0f;
}

// PostLoad and PostEditChangeProperty are now handled by ULogicEntityDataBase base class
