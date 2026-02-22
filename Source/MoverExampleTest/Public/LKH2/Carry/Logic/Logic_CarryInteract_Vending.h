// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Carry/Logic/Interface/CarryLogicInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "Logic_CarryInteract_Vending.generated.h"

class UItemData;
class AItemBase;
class AActor;
class UCarryComponent;

/**
 * 아이템 공급기(벤딩) 로직 모듈.
 * - 빈 손으로 상호작용하면 설정된 ItemData/ItemClass로 아이템을 스폰하여 지급
 * - CarryInteractComponent 위치에 공급 아이템의 Display(미리보기)를 표시
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_CarryInteract_Vending
    : public ULogicModuleBase,
      public ICarryLogicInterface {
  GENERATED_BODY()

public:
  virtual bool OnModuleInteract_Implementation(
      AActor *Interactor, AActor *TargetActor,
      ECarryInteractionType InteractionType) override;

  /** BeginPlay 시 Display 액터를 스폰하여 미리보기 세팅 */
  virtual void InitializeLogic(AActor *OwnerActor) override;

protected:
  /** 공급할 아이템의 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
  TObjectPtr<UItemData> VendingItemData;

  /** 공급할 아이템의 블루프린트 클래스 (스폰 대상) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending")
  TSubclassOf<AItemBase> VendingItemClass;

  /** Display 액터를 블랙보드에 저장할 때 사용할 GameplayTag 키 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending|Display")
  FGameplayTag DisplayActorKey;

};
