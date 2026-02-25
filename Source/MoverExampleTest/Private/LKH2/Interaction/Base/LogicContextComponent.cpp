// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "GameplayTagContainer.h"

ULogicContextComponent::ULogicContextComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = false;
}

void ULogicContextComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ULogicContextComponent, Blackboard);
}

void ULogicContextComponent::InitializeLogic(ULogicEntityDataBase *InData,
                                             AActor *Context) {
  if (!InData || !Context) {
    return;
  }

  // 이미 초기화되었고 데이터도 같다면 중복 초기화 방지
  if (bLogicInitialized && EntityData == InData) {
    return;
  }

  bLogicInitialized = true;
  EntityData = InData; // 캐싱

  TArray<ULogicModuleBase *> Modules = InData->GetAllModules();
  LogicModules.Empty(Modules.Num());

  for (ULogicModuleBase *Module : Modules) {
    if (Module) {
      ULogicModuleBase* InstancedModule = DuplicateObject<ULogicModuleBase>(Module, this);
      if (InstancedModule) {
        LogicModules.Add(InstancedModule);
        InstancedModule->InitializeLogic(Context);
      }
    }
  }
}

const FItemStatValue* ULogicContextComponent::FindStat(const FGameplayTag& Tag) const {
  // 1. 런타임 블랙보드 확인
  if (const FItemStatValue* BBStat = Blackboard.Stats.GetStat(Tag)) {
    return BBStat;
  }

  if (EntityData) {
    // 2. 엔티티 데이터 에셋 확인
    if (const FItemStatValue* EntityStat = EntityData->EntityStats.Find(Tag)) {
      return EntityStat;
    }
  }

  return nullptr;
}

void ULogicContextComponent::SetStat(const FGameplayTag& Tag, const FItemStatValue& Value) {
  if (Blackboard.Stats.SetStat(Tag, Value)) {
    // 필요한 경우 추가 후처리나 델리게이트 호출 가능
  }
}

FGameplayTag ULogicContextComponent::ResolveKey(const FGameplayTag& Key) const {
  if (const FItemStatValue* Found = FindStat(Key)) {
    if (Found->Type == EItemStatType::Tag) {
      return Found->TagValue;
    }
  }
  return Key;
}
