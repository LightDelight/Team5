// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "WorkStationBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UWorkstationData;
class UCarryInteractComponent;

UCLASS()
class MOVEREXAMPLETEST_API AWorkStationBase : public AActor,
                                              public ICarryInterface,
                                              public ILogicContextInterface {
  GENERATED_BODY()

public:
  AWorkStationBase();

  // ILogicContextInterface 구현
  virtual UCarryableComponent *GetCarryableComponent() const override;
  virtual UCarryInteractComponent *GetCarryInteractComponent() const override;
  virtual FLogicBlackboard *GetLogicBlackboard() override;

protected:
  virtual void BeginPlay() override;
  virtual void OnConstruction(const FTransform &Transform) override;

protected:
  /** 고정형 루트로 사용할 메쉬 컴포넌트 (충돌 비활성화) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UStaticMeshComponent> RootMesh;

  /** 실질적인 충돌을 담당할 박스 콜리전 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UBoxComponent> BoxCollision;

  /** 이 워크스테이션을 정의하는 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Data")
  TObjectPtr<UWorkstationData> WorkstationData;

  /** 인터랙션 위임 컴포넌트 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UCarryInteractComponent> InteractComponent;

  // ICarryInterface 구현
  // 고정형이므로 들거나 던져지지는 않지만, 인터랙션(OnCarryInteract) 및
  // 아웃라인(SetOutlineEnabled)로직에 사용
  virtual bool OnCarryInteract_Implementation(
      AActor *Interactor, ECarryInteractionType InteractionType) override;
  virtual void SetOutlineEnabled_Implementation(bool bEnabled) override;
};
