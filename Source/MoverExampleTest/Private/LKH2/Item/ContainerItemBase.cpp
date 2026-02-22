// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Item/ContainerItemBase.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Carry/Component/CarryableComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"

AContainerItemBase::AContainerItemBase() {
  InteractComponent = CreateDefaultSubobject<UCarryInteractComponent>(
      TEXT("InteractComponent"));
  // ItemBase의 RootComponent(보통 VisualMesh나 Collision)에 부착
  InteractComponent->SetupAttachment(RootComponent);
}

bool AContainerItemBase::OnCarryInteract_Implementation(
    AActor *Interactor, ECarryInteractionType InteractionType) {
  // 1. 플레이어 컨텍스트 확인 (이미 무언가를 들고 있는지?)
  bool bIsHoldingItem = false;
  if (Interactor && Interactor->Implements<UInstigatorContextInterface>()) {
    UCarryComponent *InstigatorCarryComp =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
    if (InstigatorCarryComp) {
      bIsHoldingItem = (InstigatorCarryComp->GetCarriedActor() != nullptr);
    }
  }

  // 2. 동적 라우팅 로직
  // 최우선 순위: 플레이어가 무언가를 들고 상호작용했다면
  // 워크스테이션(담기/조합) 의도로 간주
  if (bIsHoldingItem && InteractComponent && WorkstationData) {
    if (InteractComponent->OnInteract(Interactor, WorkstationData,
                                      InteractionType)) {
      // 워크스테이션 로직이 성공적으로 처리되었으므로 종료
      return true;
    }
  }

  // 3. 폴백 (Fallback): 플레이어 손이 비었거나, 워크스테이션 로직이
  // 거부(false)한 경우 -> 물건 줍기 시도
  if (CarryComponent) {
    return CarryComponent->OnCarryInteract(Interactor, InteractionType);
  }

  return false;
}

UCarryableComponent *AContainerItemBase::GetCarryableComponent() const {
  return CarryComponent;
}

UCarryInteractComponent *AContainerItemBase::GetCarryInteractComponent() const {
  return InteractComponent;
}

FLogicBlackboard *AContainerItemBase::GetLogicBlackboard() {
  if (InteractComponent) {
    return &InteractComponent->LogicBlackboard;
  }
  return nullptr;
}
