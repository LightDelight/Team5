// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"

UInteractableComponent::UInteractableComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  bIsInteracting = false;
  CurrentInteractor = nullptr;
}

void UInteractableComponent::BeginPlay() { Super::BeginPlay(); }

void UInteractableComponent::InitializeLogic(ULogicEntityDataBase *InData,
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

bool UInteractableComponent::OnInteract(const FInteractionContext &Context) {
  if (Context.Interactor) {
    // 자신(Interactable의 Owner)을 TargetActor로 보장하는 Context 사본 생성
    FInteractionContext ModifiedContext = Context;
    
    // 만약 TargetActor가 지정되지 않았다면 자기 자신으로 설정
    if (ModifiedContext.TargetActor == nullptr) {
        ModifiedContext.TargetActor = GetOwner();
    }

    // 피시도자 측 컴포넌트 세팅
    if (AActor* OwnerActor = GetOwner()) {
      ModifiedContext.InteractableComp = this;
      ModifiedContext.InteractablePropertyComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>();
      ModifiedContext.ContextComp = OwnerActor->FindComponentByClass<ULogicContextComponent>();
    }

    // 이제 내부에서 직접 LogicModules 배열을 순회합니다.
    for (ULogicModuleBase *Module : LogicModules) {
      if (Module) {
        if (Module->ExecuteInteraction(ModifiedContext)) {
          return true; // 처리에 성공하면 흐름을 중단
        }
      }
    }
  }
  return false;
}


void UInteractableComponent::SetOutlineEnabled(bool bEnabled) {
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