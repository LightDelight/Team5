// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Cart_Snap.generated.h"

/**
 * 카트의 Box Collision Overlap 이벤트(Intent.Cart.ItemOverlap)를 수신하여
 * 아이템을 카트에 스냅(Stored)하는 로직 모듈.
 *
 * Context.TargetActor에 Overlap된 액터(AItemBase)가 담겨 있어야 합니다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced,
       meta = (DisplayName = "Logic : Cart Snap"))
class MOVEREXAMPLETEST_API ULogic_Cart_Snap : public ULogicModuleBase
{
    GENERATED_BODY()

public:
    ULogic_Cart_Snap();

    virtual bool PreInteractCheck(const FInteractionContext& Context) override;
    virtual bool PerformInteraction(const FInteractionContext& Context) override;

    /**
     * 아이템을 보관할 슬롯 태그 목록.
     * 순서대로 비어있는 첫 번째 슬롯에 저장합니다.
     * 예: [Cart.Slot.0, Cart.Slot.1, Cart.Slot.2, ...]
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|CartSnap")
    TArray<FGameplayTag> SlotTags;

    /** 카트가 수용할 수 있는 최대 아이템 수 (SlotTags 배열 크기로 자동 확인) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|CartSnap",
              meta = (ClampMin = "1"))
    int32 MaxItemCount = 6;
};
