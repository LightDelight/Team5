// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Item/ItemBase.h"
#include "ContainerItemBase.generated.h"

class UCarryInteractComponent;

/**
 * 들고 다닐 수 있으면서(Carryable), 동시에 다른 아이템을 담거나 조합할 수
 * 있는(Workstation) 복합 상호작용 아이템의 기반 클래스입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AContainerItemBase : public AItemBase {
  GENERATED_BODY()

public:
  AContainerItemBase();

  virtual bool OnCarryInteract_Implementation(const FCarryContext &Context) override;

  // 모듈 생명주기 연결
  virtual void BeginPlay() override;
  virtual void OnConstruction(const FTransform &Transform) override;

  // ILogicContextInterface 구현
  virtual UCarryInteractComponent *GetCarryInteractComponent() const override;

protected:
  /** 접시나 바구니처럼 다른 아이템을 받을 수 있게 해주는 상호작용 컴포넌트 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
  TObjectPtr<UCarryInteractComponent> InteractComponent;
};
