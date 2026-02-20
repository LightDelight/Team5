// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/CarryInteractComponent.h"
#include "LKH2/CarryInteractLogicInterface.h"
#include "LKH2/LogicModuleBase.h"
#include "LKH2/WorkstationData.h"

#include "Net/UnrealNetwork.h"

UCarryInteractComponent::UCarryInteractComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarryInteractComponent::BeginPlay() { Super::BeginPlay(); }

void UCarryInteractComponent::OnInteract(AActor *Interactor,
                                         UWorkstationData *Data) {
  // Actor로부터 메시지 전달 시 WorkstationData가 가진 로직 호출
  if (Data && Interactor) {
    for (ULogicModuleBase *Module : Data->LogicModules) {
      if (Module && Module->Implements<UCarryInteractLogicInterface>()) {
        ICarryInteractLogicInterface::Execute_OnModuleInteract(
            Module, Interactor, GetOwner());
      }
    }
  }
}
