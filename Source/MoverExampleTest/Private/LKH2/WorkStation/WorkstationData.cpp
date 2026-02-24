// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/WorkStation/WorkstationData.h"
#include "LKH2/Data/PresetData.h"
#include "LKH2/Data/VisualPresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

TArray<ULogicModuleBase *> UWorkstationData::GetAllModules() const {
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
  PresetStats.Empty();
  PresetRequiredTags.Empty();

  if (Preset) {
    PresetModules = Preset->DefaultModuleClasses;
    PresetStats = Preset->DefaultStats;
    PresetRequiredTags = Preset->RequiredStatTags;
  }
}

void UWorkstationData::PostLoad() {
  Super::PostLoad();
  RefreshPresetPreview();
}

#if WITH_EDITOR
void UWorkstationData::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
  RefreshPresetPreview();
}
#endif
