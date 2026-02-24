// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Data/PresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

void UPresetData::RefreshRequiredTags() {
  RequiredStatTags.Empty();

  for (const TSubclassOf<ULogicModuleBase> &ModuleClass :
       DefaultModuleClasses) {
    if (ModuleClass) {
      ULogicModuleBase *CDO =
          ModuleClass->GetDefaultObject<ULogicModuleBase>();
      if (CDO) {
        for (const FGameplayTag &Tag : CDO->GetRequiredStatTags()) {
          RequiredStatTags.AddUnique(Tag);
        }
      }
    }
  }
}

void UPresetData::PostLoad() {
  Super::PostLoad();
  RefreshRequiredTags();
}

#if WITH_EDITOR
void UPresetData::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
  RefreshRequiredTags();
}
#endif
