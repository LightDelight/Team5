// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "ContainerItemBase.generated.h"

class UCarryInteractComponent;
class UWorkstationData;

/**
 * 들고 다닐 수 있으면서(Carryable), 동시에 다른 아이템을 담거나 조합할 수
 * 있는(Workstation) 복합 상호작용 아이템의 기반 클래스입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AContainerItemBase : public AItemBase,
                                                public ILogicContextInterface {
  GENERATED_BODY()

public:
  AContainerItemBase();

  virtual bool OnCarryInteract_Implementation(
      AActor *Interactor, ECarryInteractionType InteractionType) override;

  // 모듈 생명주기 연결
  virtual void BeginPlay() override;
  virtual void OnConstruction(const FTransform &Transform) override;

  // ILogicContextInterface 구현
  virtual UCarryableComponent *GetCarryableComponent() const override;
  virtual UCarryInteractComponent *GetCarryInteractComponent() const override;
  virtual FLogicBlackboard *GetLogicBlackboard() override;

protected:
  /** 접시나 바구니처럼 다른 아이템을 받을 수 있게 해주는 상호작용 컴포넌트 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
  TObjectPtr<UCarryInteractComponent> InteractComponent;

  /** 이 컨테이너가 처리할 수 있는 로직 모듈들을 정의하는 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container|Data")
  TObjectPtr<UWorkstationData> WorkstationData;
};
