// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"

TArray<ULogicModuleBase *> ULogicEntityDataBase::GetAllModules() const {
  TArray<ULogicModuleBase *> AllModules;

  // 추가 모듈 (Instanced)
  for (const TObjectPtr<ULogicModuleBase> &Module : AdditionalModules) {
    if (Module) {
      AllModules.Add(Module.Get());
    }
  }

  return AllModules;
}

