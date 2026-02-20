// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/CarryInteractLogicInterface.h"
#include "LKH2/LogicModuleBase.h"
#include "Logic_CarryInteract_Common.generated.h"

/**
 * 워크스테이션에 아이템을 올려놓거나 다시 줍는(반납하는) 로직을 담당하는 공통
 * 모듈. 한 번에 하나의 아이템만 올려놓을 수 있도록 관리.
 */
UCLASS(Blueprintable, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_CarryInteract_Common
    : public ULogicModuleBase,
      public ICarryInteractLogicInterface {
  GENERATED_BODY()

public:
  ULogic_CarryInteract_Common();

  virtual void
  OnModuleInteract_Implementation(AActor *Interactor,
                                  AActor *TargetWorkstation) override;

private:
  // 현재 워크스테이션에 거치된 아이템
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact",
            ReplicatedUsing = OnRep_StoredItem,
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<AActor> StoredItem;

  UFUNCTION()
  void OnRep_StoredItem();

public:
  virtual bool IsSupportedForNetworking() const override { return true; }
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};
