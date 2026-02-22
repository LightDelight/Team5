// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Logic/Logic_Carryable_Common.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Logic/InstigatorContextInterface.h"

#include "LKH2/Item/ItemStateComponent.h"

bool ULogic_Carryable_Common::OnModuleInteract_Implementation(
    AActor *Interactor, AActor *TargetActor,
    ECarryInteractionType InteractionType) {
  if (!TargetActor || !Interactor)
    return false;

  UCarryComponent *Carrier = nullptr;
  if (Interactor->Implements<UInstigatorContextInterface>()) {
    Carrier =
        IInstigatorContextInterface::Execute_GetCarryComponent(Interactor);
  }

  UItemStateComponent *StateComp =
      TargetActor->FindComponentByClass<UItemStateComponent>();
  if (!StateComp)
    return false;

  bool bIsCarried = (StateComp->CurrentState == EItemState::Carried);

  if (!bIsCarried) {
    // ----------------------------------------------------
    // 줍기 (Pick Up: Placed/Dropped/Stored -> Carried)
    // ----------------------------------------------------
    if (Carrier && Carrier->GetCarriedActor() != nullptr) {
      return false; // 이미 다른 것을 들고 있으면 실패
    }

    if (TargetActor->HasAuthority()) {
      StateComp->SetItemState(EItemState::Carried);

      if (UPrimitiveComponent *RootPrim =
              Cast<UPrimitiveComponent>(TargetActor->GetRootComponent())) {
        RootPrim->SetSimulatePhysics(false);

        if (Carrier) {
          TargetActor->AttachToComponent(
              Carrier,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        } else {
          TargetActor->AttachToActor(
              Interactor,
              FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }
      }
    } else {
      // 클라이언트 예측 (Prediction)
      StateComp->CurrentState = EItemState::Carried;
      if (UPrimitiveComponent *RootPrim =
              Cast<UPrimitiveComponent>(TargetActor->GetRootComponent())) {
        RootPrim->SetSimulatePhysics(false);
        RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
      }
    }
    return true;
  } else {
    // ----------------------------------------------------
    // 내려놓기 또는 던지기 (Drop/Throw: Carried -> Placed)
    // ----------------------------------------------------
    if (TargetActor->HasAuthority()) {
      StateComp->SetItemState(EItemState::Placed);

      TargetActor->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
      if (UPrimitiveComponent *RootPrim =
              Cast<UPrimitiveComponent>(TargetActor->GetRootComponent())) {
        RootPrim->SetSimulatePhysics(true);

        // 던지기 판단: CarryComponent에 있던 물리 제어를 여기서 수행
        if (InteractionType == ECarryInteractionType::Throw) {
          FVector ThrowVelocity = Interactor->GetActorForwardVector() * 800.0f +
                                  FVector(0, 0, 300.0f); // 던지기 물리
          RootPrim->AddImpulse(ThrowVelocity, NAME_None, true);
        }
      }
    } else {
      // 클라이언트 예측 (Prediction)
      StateComp->CurrentState = EItemState::Placed;

      TargetActor->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
      if (UPrimitiveComponent *RootPrim =
              Cast<UPrimitiveComponent>(TargetActor->GetRootComponent())) {
        RootPrim->SetSimulatePhysics(true);
        RootPrim->SetCollisionProfileName(TEXT("PhysicsActor"));
      }
    }
    return true;
  }
}
