// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/WorkStation/WorkstationData.h"
#include "LKH2/Data/PresetData.h"
#include "LKH2/Logic/LogicModuleBase.h"

TArray<ULogicModuleBase *> UWorkstationData::GetAllModules() const {
  TArray<ULogicModuleBase *> Result;

  // Track 1: Preset 모듈 (CDO 기반)
  if (Preset) {
    for (const TSubclassOf<ULogicModuleBase> &ModuleClass :
         Preset->DefaultModuleClasses) {
      if (ModuleClass) {
        // CDO(Class Default Object)를 사용 — stateless 모듈이므로 안전
        ULogicModuleBase *CDO = ModuleClass->GetDefaultObject<ULogicModuleBase>();
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
