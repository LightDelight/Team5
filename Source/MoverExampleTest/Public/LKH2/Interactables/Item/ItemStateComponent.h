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

  /**
   * 아이템의 상태를 Dropped로 변경하고 임펄스를 가합니다.
   * 물리 및 충돌 활성화 처리는 SetItemState(Dropped) 내에서 처리됩니다.
   *
   * @param Impulse 가해질 던지기 힘
   */
  UFUNCTION(BlueprintCallable, Category = "Item State")
  void ThrowItem(const FVector &Impulse);
};
