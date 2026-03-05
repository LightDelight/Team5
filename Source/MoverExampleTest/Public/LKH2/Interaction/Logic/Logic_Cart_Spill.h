// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Cart_Spill.generated.h"

class UMarkerData;

/**
 * 카트 전복 이벤트(Intent.Cart.Overturn)를 수신하여 카트 내 모든 아이템을
 * Spilled 상태로 전환하고, 정리 마커를 스폰하는 로직 모듈.
 *
 * 스폰된 마커에 아이템 UID를 담은 Context를 전송하여
 * Logic_Holding_SpillCleanup이 정리 작업을 수행할 수 있도록 합니다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced,
       meta = (DisplayName = "Logic : Cart Spill"))
class MOVEREXAMPLETEST_API ULogic_Cart_Spill : public ULogicModuleBase
{
    GENERATED_BODY()

public:
    ULogic_Cart_Spill();

    virtual bool PreInteractCheck(const FInteractionContext& Context) override;
    virtual bool PerformInteraction(const FInteractionContext& Context) override;

    /**
     * 쏟기 발생 시 스폰할 정리 마커 DA.
     * 에디터에서 Logic_Holding_SpillCleanup 모듈을 가진 MarkerData를 꽂아 사용.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|CartSpill")
    TObjectPtr<UMarkerData> CleanupMarkerDA;

    /** 아이템 쏟기 시 부여할 랜덤 임펄스 최대 크기 (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|CartSpill",
              meta = (ClampMin = "0.0"))
    float SpillImpulseStrength = 300.0f;

    /** 정리 마커를 카트 기준 어느 위치에 스폰할지 오프셋 (로컬 좌표) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|CartSpill")
    FVector MarkerSpawnOffset = FVector(0.0f, 0.0f, 0.0f);
};
