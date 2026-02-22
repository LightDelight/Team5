// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Logic/Interface/CarryLogicInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "LKH2/WorkStation/WorkstationData.h"

#include "Net/UnrealNetwork.h"

UCarryInteractComponent::UCarryInteractComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarryInteractComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(UCarryInteractComponent, LogicBlackboard);
}

void UCarryInteractComponent::BeginPlay() { Super::BeginPlay(); }

bool UCarryInteractComponent::OnInteract(
    AActor *Interactor, UWorkstationData *Data,
    ECarryInteractionType InteractionType) 
    {
  // Actor로부터 메시지 전달 시 WorkstationData가 가진 로직 호출
  if (Data && Interactor) {
    // 로직 모듈 배열을 순회하며 하나씩 실행 (책임 연쇄 패턴)
    for (ULogicModuleBase *Module : Data->LogicModules) {
      if (Module && Module->Implements<UCarryLogicInterface>()) {
        if (ICarryLogicInterface::Execute_OnModuleInteract(
                Module, Interactor, GetOwner(), InteractionType)) {
          return true; // 처리에 성공하면 흐름을 중단
        }
      }
    }
  }
  return false;
}
