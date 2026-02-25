// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interaction/Base/LogicInteractionInterface.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_WorkstationInteract_Common.generated.h"

// Forward declaration
class UItemData;
class UWorkstationData;

/**
 * 워크스테이션(혹은 바닥 거치대)처럼 아이템을 올리고 내리는 로직
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_WorkstationInteract_Common
    : public ULogicModuleBase {
  GENERATED_BODY()

public:
  ULogic_WorkstationInteract_Common();

  virtual bool PreInteractCheck(const FInteractionContext &Context) override;
  virtual bool PerformInteraction(const FInteractionContext &Context) override;

protected:
  /** 이 로직이 블랙보드에 아이템을 거치할 때 사용할 GameplayTag 키입니다.
   * 에디터에서 선택합니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Mapping")
  FGameplayTag StoredItemKey;

  /** 플레이어가 아이템을 들고 있을 때 거치(Store) 동작을 허용할지 여부입니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic Constraints")
  bool bAllowStore = true;

  /** 플레이어 손이 비었을 때 거치된 아이템 회수(Retrieve) 동작을 허용할지 여부입니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic Constraints")
  bool bAllowRetrieve = true;
};
