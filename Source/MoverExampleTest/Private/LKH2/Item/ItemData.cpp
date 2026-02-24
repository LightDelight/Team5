// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Item/ItemData.h"
#include "LKH2/Data/PresetData.h"
#include "LKH2/Data/VisualPresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

TArray<ULogicModuleBase *> UItemData::GetAllModules() const {
  TArray<ULogicModuleBase *> Result;

  // Track 1: Preset 모듈 (CDO 기반)
  if (Preset) {
    for (const TSubclassOf<ULogicModuleBase> &ModuleClass :
         Preset->DefaultModuleClasses) {
      if (ModuleClass) {
        ULogicModuleBase *CDO =
            ModuleClass->GetDefaultObject<ULogicModuleBase>();
        if (CDO) {
          Result.Add(CDO);
        }
      }
    }
  }

  // Track 2: 추가 모듈 (Instanced)
  for (const TObjectPtr<ULogicModuleBase> &Module : AdditionalModules) {
    if (Module) {
      Result.Add(Module.Get());
    }
  }

  return Result;
}

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
  if (const FItemStatValue *Found = ItemStats.Find(WeightTag)) {
    if (Found->Type == EItemStatType::Float) {
      return Found->FloatValue;
    }
  }

  // 2. Preset DefaultStats에서 찾기
  if (Preset) {
    if (const FItemStatValue *Found = Preset->DefaultStats.Find(WeightTag)) {
      if (Found->Type == EItemStatType::Float) {
        return Found->FloatValue;
      }
    }
  }

  // 3. 하드코딩 기본값
  return 10.0f;
}

// ─── Preset 미리보기 갱신 ───

void UItemData::RefreshPresetPreview() {
  PresetModules.Empty();
  PresetStats.Empty();
  PresetRequiredTags.Empty();

  if (Preset) {
    PresetModules = Preset->DefaultModuleClasses;
    PresetStats = Preset->DefaultStats;
    PresetRequiredTags = Preset->RequiredStatTags;
  }
}

void UItemData::PostLoad() {
  Super::PostLoad();
  RefreshPresetPreview();
}

#if WITH_EDITOR
void UItemData::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
  RefreshPresetPreview();
}
#endif
