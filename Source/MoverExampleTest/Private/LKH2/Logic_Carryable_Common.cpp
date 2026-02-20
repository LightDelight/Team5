// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Logic_Carryable_Common.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/CarryComponent.h"

void ULogic_Carryable_Common::OnModulePickedUp_Implementation(
    AActor *Carrier, AActor *ItemTarget) {
  // 일반적인 줍기 로직: Carrier의 특정 컴포넌트(예: 손이나 CarryComponent의
  // 자식)로 ItemTarget 부착 (AttachToComponent) 여기서는 단순히 Actor를
  // 들었다는 로그나 기본적인 물리 비활성화를 수행할 수 있습니다.

  if (ItemTarget && Carrier) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(ItemTarget->GetRootComponent())) {
      RootPrim->SetSimulatePhysics(false);

      if (UCarryComponent *CarryComp =
              Carrier->FindComponentByClass<UCarryComponent>()) {
        ItemTarget->AttachToComponent(
            CarryComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            AttachmentSocketName);
      } else {
        ItemTarget->AttachToActor(
            Carrier, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            AttachmentSocketName);
      }
    }
  }
}

void ULogic_Carryable_Common::OnModuleDropped_Implementation(
    AActor *Carrier, AActor *ItemTarget) {
  // 일반적인 놓기 로직: 부착 해제 및 물리 시뮬레이션 활성화

  if (ItemTarget) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(ItemTarget->GetRootComponent())) {
      ItemTarget->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
      RootPrim->SetSimulatePhysics(true);
    }
  }
}

void ULogic_Carryable_Common::OnModuleThrown_Implementation(
    AActor *Carrier, AActor *ItemTarget, FVector ThrowVelocity) {
  // 일반적인 던지기 로직: 부착 해제, 물리 시뮬레이션 활성화 및 임펄스(Impulse)
  // 적용

  if (ItemTarget) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(ItemTarget->GetRootComponent())) {
      ItemTarget->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
      RootPrim->SetSimulatePhysics(true);
      RootPrim->AddImpulse(ThrowVelocity, NAME_None,
                           true); // true = VelocityChange(질량 무시)
    }
  }
}
