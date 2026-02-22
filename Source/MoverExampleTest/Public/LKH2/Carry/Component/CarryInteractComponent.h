// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Logic/LogicBlackboard.h"
#include "CarryInteractComponent.generated.h"


// 전방 선언
class UWorkstationData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCarryInteractComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UCarryInteractComponent();

  // 상호작용 처리 함수. Actor로부터 메시지 전달 시 호출됨.
  bool OnInteract(AActor *Interactor, UWorkstationData *Data,
                  ECarryInteractionType InteractionType);

  /** 로직 상태를 관리하고 멀티플레이 복제를 담당하는 블랙보드입니다. */
  UPROPERTY(Replicated)
  FLogicBlackboard LogicBlackboard;

protected:
  virtual void BeginPlay() override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};
