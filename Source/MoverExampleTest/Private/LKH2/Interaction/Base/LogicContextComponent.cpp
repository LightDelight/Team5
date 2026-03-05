// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interaction/Base/LogicTaskBase.h"
#include "GameplayTagContainer.h"

ULogicContextComponent::ULogicContextComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = false;
}

void ULogicContextComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ULogicContextComponent, Stats);
  DOREPLIFETIME(ULogicContextComponent, ObjectBlackboard);
}

// [Moved] InitializeLogic implementation has been moved to UInteractableComponent.

const FItemStatValue* ULogicContextComponent::FindStat(const FGameplayTag& Tag) const {
  // 1. 런타임 블랙보드 확인
  if (const FItemStatValue* BBStat = Stats.GetStat(Tag)) {
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
  if (Stats.SetStat(Tag, Value)) {
    // 필요한 경우 추가 후처리나 델리게이트 호출 가능
  }
}

void ULogicContextComponent::RemoveStat(FGameplayTag Tag) {
  Stats.RemoveStat(Tag);
}

void ULogicContextComponent::ClearAllStats() {
  Stats.ClearStats();
}

void ULogicContextComponent::RemoveObject(FGameplayTag Tag) {
  ObjectBlackboard.RemoveObject(Tag);
}

void ULogicContextComponent::ClearAllObjects() {
  ObjectBlackboard.ClearObjects();
}

FGameplayTag ULogicContextComponent::ResolveKey(const FGameplayTag& Key) const
{
	if (const FItemStatValue* Found = FindStat(Key)) {
		if (Found->Type == EItemStatType::Tag) {
			return Found->TagValue;
		}
	}
	return Key;
}

void ULogicContextComponent::SetTask(FGameplayTag TaskTag, ULogicTaskBase* NewTask)
{
	if (NewTask)
	{
		ActiveTasks.Add(TaskTag, NewTask);
	}
}

ULogicTaskBase* ULogicContextComponent::GetTask(FGameplayTag TaskTag) const
{
	if (const TObjectPtr<ULogicTaskBase>* FoundTask = ActiveTasks.Find(TaskTag))
	{
		return *FoundTask;
	}
	return nullptr;
}

void ULogicContextComponent::ClearTask(FGameplayTag TaskTag)
{
	ActiveTasks.Remove(TaskTag);
}
