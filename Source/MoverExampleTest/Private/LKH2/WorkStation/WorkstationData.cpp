// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/WorkStation/WorkstationData.h"
#include "LKH2/Data/LogicEntityDataBase.h"
#include "LKH2/Data/PresetData.h"
#include "LKH2/Data/VisualPresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

// GetAllModules is now handled by ULogicEntityDataBase base class

// ─── 비주얼 헬퍼 ───

TSubclassOf<AWorkStationBase> UWorkstationData::GetEffectiveWorkstationClass() const {
  if (bUseCustomVisuals && CustomWorkstationClass) {
    return CustomWorkstationClass;
  }
  return VisualPreset ? VisualPreset->DefaultWorkstationClass : nullptr;
}

UStaticMesh *UWorkstationData::GetEffectiveWorkstationMesh() const {
  if (bUseCustomVisuals && CustomWorkstationMesh) {
    return CustomWorkstationMesh;
  }
  return VisualPreset ? VisualPreset->DefaultWorkstationMesh.Get() : nullptr;
}

FVector UWorkstationData::GetEffectiveBoxExtent() const {
  if (bUseCustomVisuals) {
    return CustomBoxExtent;
  }
  return VisualPreset ? VisualPreset->DefaultBoxExtent
                      : FVector(50.0f, 50.0f, 50.0f);
}

FVector UWorkstationData::GetEffectiveBoxRelativeLocation() const {
  if (bUseCustomVisuals) {
    return CustomBoxRelativeLocation;
  }
  return VisualPreset ? VisualPreset->DefaultBoxRelativeLocation
                      : FVector::ZeroVector;
}

FVector UWorkstationData::GetEffectiveInteractRelativeLocation() const {
  if (bUseCustomVisuals) {
    return CustomInteractRelativeLocation;
  }
  return VisualPreset ? VisualPreset->DefaultInteractRelativeLocation
                      : FVector::ZeroVector;
}

// ─── Preset 미리보기 갱신 ───

void UWorkstationData::RefreshPresetPreview() {
  PresetModules.Empty();
  WorkstationPresetStats.Empty();
  WorkstationRequiredTags.Empty();

  if (Preset) {
    PresetModules = Preset->DefaultModuleClasses;
    WorkstationPresetStats = Preset->DefaultStats;
    WorkstationRequiredTags = Preset->RequiredStatTags;
  }
}

// PostLoad and PostEditChangeProperty are now handled by ULogicEntityDataBase base class
