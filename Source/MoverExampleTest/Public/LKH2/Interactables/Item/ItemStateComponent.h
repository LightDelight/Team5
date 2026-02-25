// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "ItemStateComponent.generated.h"

UENUM(BlueprintType)
enum class EItemState : uint8 {
  Placed UMETA(DisplayName = "Placed"),
  Carried UMETA(DisplayName = "Carried"),
  Dropped UMETA(DisplayName = "Dropped"),
  Stored UMETA(DisplayName = "Stored")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UItemStateComponent : public UActorComponent {
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UItemStateComponent();

protected:
  // Called when the game starts
  virtual void BeginPlay() override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

public:
  // Called every frame
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  UPROPERTY(ReplicatedUsing = OnRep_ItemState, BlueprintReadOnly,
            Category = "Item State")
  EItemState CurrentState;

  UFUNCTION()
  void OnRep_ItemState();

  UFUNCTION(BlueprintCallable, Category = "Item State")
  void SetItemState(EItemState NewState);
};
