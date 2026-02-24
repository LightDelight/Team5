// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Data/LogicEntityDataBase.h"
#include "LKH2/Data/PresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

TArray<ULogicModuleBase *> ULogicEntityDataBase::GetAllModules() const {
  TArray<ULogicModuleBase *> AllModules;

  // Track 1: Preset 모듈 (CDO 기반)
  if (Preset) {
    for (const TSubclassOf<ULogicModuleBase> &ModuleClass :
         Preset->DefaultModuleClasses) {
      if (ModuleClass) {
        if (ULogicModuleBase *CDO =
                ModuleClass->GetDefaultObject<ULogicModuleBase>()) {
          AllModules.Add(CDO);
        }
      }
    }
  }

  // Track 2: 추가 모듈 (Instanced)
  for (const TObjectPtr<ULogicModuleBase> &Module : AdditionalModules) {
    if (Module) {
      AllModules.Add(Module.Get());
    }
  }

  return AllModules;
}

#if WITH_EDITOR
void ULogicEntityDataBase::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
  RefreshPresetPreview();
}
#endif

void ULogicEntityDataBase::PostLoad() {
  Super::PostLoad();
  RefreshPresetPreview();
}
