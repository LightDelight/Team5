// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemSmoothingComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UItemSmoothingComponent::UItemSmoothingComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PostPhysics; // 물리 처리 후 보간
}

// Called when the game starts
void UItemSmoothingComponent::BeginPlay() { Super::BeginPlay(); }

// Called every frame
void UItemSmoothingComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (bUseDeadReckoning && TargetRoot && InterpolatedVisual) {
    FVector CurrentLoc = InterpolatedVisual->GetComponentLocation();
    FVector TargetLoc = TargetRoot->GetComponentLocation();

    FRotator CurrentRot = InterpolatedVisual->GetComponentRotation();
    FRotator TargetRot = TargetRoot->GetComponentRotation();

    FVector NewLoc =
        FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, LocationInterpSpeed);
    FRotator NewRot =
        FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);

    InterpolatedVisual->SetWorldLocationAndRotation(NewLoc, NewRot);
  }
}

void UItemSmoothingComponent::InitialSetup(USceneComponent *InRoot,
                                           USceneComponent *InVisual) {
  TargetRoot = InRoot;
  InterpolatedVisual = InVisual;

  if (InterpolatedVisual && TargetRoot && bUseDeadReckoning) {
    // 보간을 위해 비주얼 메쉬를 루트에서 분리 (월드에 남김)
    InterpolatedVisual->DetachFromComponent(
        FDetachmentTransformRules::KeepWorldTransform);
  }
}
