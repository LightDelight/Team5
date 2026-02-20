// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "ItemSmoothingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UItemSmoothingComponent : public UActorComponent {
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UItemSmoothingComponent();

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
  float LocationInterpSpeed = 20.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
  float RotationInterpSpeed = 20.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
  bool bUseDeadReckoning = true;

  void InitialSetup(USceneComponent *InRoot, USceneComponent *InVisual);

private:
  TObjectPtr<USceneComponent> TargetRoot;
  TObjectPtr<USceneComponent> InterpolatedVisual;
};
