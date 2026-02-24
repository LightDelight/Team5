// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "CarryInteractComponent.generated.h"


// 전방 선언
class UWorkstationData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCarryInteractComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UCarryInteractComponent();

  // 상호작용 처리 함수. Actor로부터 메시지 전달 시 호출됨.
  bool OnInteract(const FCarryContext &Context);

protected:
  // [Pull Pattern] 소유자 인터페이스를 통해 모듈을 조회하므로 자체 배열은 더 이상 사용하지 않습니다.


protected:
  virtual void BeginPlay() override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};
