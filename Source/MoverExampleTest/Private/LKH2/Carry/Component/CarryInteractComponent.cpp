// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "LKH2/WorkStation/WorkstationData.h"

#include "Net/UnrealNetwork.h"

UCarryInteractComponent::UCarryInteractComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarryInteractComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UCarryInteractComponent::BeginPlay() { Super::BeginPlay(); }

bool UCarryInteractComponent::OnInteract(const FCarryContext &Context) {
  if (Context.Interactor) {
    // [Pull Pattern] 소유자로부터 인터페이스를 통해 최신 모듈 목록을 직접 조회
    ILogicContextInterface *LogicContext = Cast<ILogicContextInterface>(GetOwner());
    if (LogicContext) {
      TArray<ULogicModuleBase *> Modules = LogicContext->GetLogicModules();
      for (ULogicModuleBase *Module : Modules) {
        if (Module) {
          if (Module->ExecuteInteraction(Context)) {
            return true; // 처리에 성공하면 흐름을 중단
          }
        }
      }
    }
  }
  return false;
}

