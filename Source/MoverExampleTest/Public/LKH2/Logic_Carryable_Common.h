// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/CarryableLogicInterface.h"
#include "LKH2/LogicModuleBase.h"
#include "Logic_Carryable_Common.generated.h"


/**
 * 일반적인 줍기, 놓기, 던지기 로직을 담당하는 공통 모듈
 */
UCLASS(Blueprintable, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_Carryable_Common
    : public ULogicModuleBase,
      public ICarryableLogicInterface {
  GENERATED_BODY()

public:
  // ICarryableLogicInterface 구현
  virtual void OnModulePickedUp_Implementation(AActor *Carrier,
                                               AActor *ItemTarget) override;
  virtual void OnModuleDropped_Implementation(AActor *Carrier,
                                              AActor *ItemTarget) override;
  virtual void OnModuleThrown_Implementation(AActor *Carrier,
                                             AActor *ItemTarget,
                                             FVector ThrowVelocity) override;

public:
  /** 아이템을 줍을 때 부착할 캐릭터의 소켓 이름 (예: Hand_R) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  FName AttachmentSocketName = NAME_None;
};
