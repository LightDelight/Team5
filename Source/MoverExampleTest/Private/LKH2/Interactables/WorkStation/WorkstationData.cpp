// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/WorkStation/WorkstationData.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interactables/Data/VisualPresetData.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"

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

// ─── Preset 미리보기 갱신 없음 ───

// PostLoad and PostEditChangeProperty are now handled by ULogicEntityDataBase base class
