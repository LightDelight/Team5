// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CarryInteractComponent.generated.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"


// 전방 선언
class UWorkstationData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCarryInteractComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UCarryInteractComponent();

  // 상호작용 처리 함수. Actor로부터 메시지 전달 시 호출됨.
  void OnInteract(AActor *Interactor, UWorkstationData *Data);

protected:
  virtual void BeginPlay() override;
};
