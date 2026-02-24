// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "Logic_CarryInteract_Vending.generated.h"

class UItemData;
class AItemBase;
class AActor;
class UCarryComponent;

/**
 * 아이템 공급기(벤딩) 로직 모듈.
 * - 빈 손으로 상호작용하면 Stats에서 조회한 ItemData로 아이템을 스폰
 * - 스폰 클래스는 ItemData의 VisualPreset에서 가져옴
 * - CarryInteractComponent 위치에 공급 아이템의 Display(미리보기)를 표시
 *
 * 필요 Stats 태그:
 *   - VendingItemDataTag (Object → UItemData*)
 *   - DisplayActorKey (블랙보드 키)
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class MOVEREXAMPLETEST_API ULogic_CarryInteract_Vending
    : public ULogicModuleBase {
  GENERATED_BODY()

public:
  virtual bool PreInteractCheck(const FCarryContext &Context) override;
  virtual bool PerformInteraction(const FCarryContext &Context) override;

  /** BeginPlay 시 Display 액터를 스폰하여 미리보기 세팅 */
  virtual void InitializeLogic(AActor *InOwnerActor) override;

  /** 필수 Stats 태그 선언 */
  virtual TArray<FGameplayTag> GetRequiredStatTags() const override;

protected:
  // ─── 필요 태그 선언 ───

  /** Stats에서 공급 아이템 태그(FGameplayTag)를 조회할 태그 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Vending|RequiredTags")
  FGameplayTag VendingItemTag;

  /** Display 액터를 블랙보드에 저장할 때 사용할 GameplayTag 키.
   *  ResolveKey를 통해 치환될 수 있습니다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vending|Display")
  FGameplayTag DisplayActorKey;
};
