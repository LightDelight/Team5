// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Component/CarryableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Logic/LogicModuleBase.h"

UCarryableComponent::UCarryableComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  bIsCarried = false;
  CurrentCarrier = nullptr;
}

void UCarryableComponent::BeginPlay() { Super::BeginPlay(); }

bool UCarryableComponent::OnCarryInteract(const FCarryContext &Context) {
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


void UCarryableComponent::SetOutlineEnabled(bool bEnabled) {
  if (AActor *OwnerActor = GetOwner()) {
    TArray<UPrimitiveComponent *> PrimitiveComps;
    OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
    for (UPrimitiveComponent *Comp : PrimitiveComps) {
      if (Comp) {
        Comp->SetRenderCustomDepth(bEnabled);
        Comp->SetCustomDepthStencilValue(1); // 외곽선을 위한 스텐실 값
      }
    }
  }
}
