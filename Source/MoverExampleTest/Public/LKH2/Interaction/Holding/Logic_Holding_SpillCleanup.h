#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Holding/HoldingStampLogicModuleBase.h"
#include "Logic_Holding_SpillCleanup.generated.h"

/**
 * 카트 전복 정리 작업을 위한 Stamp 홀딩 로직 모듈.
 * 정리 마커 DA의 AdditionalModules에 꽂아 사용합니다.
 *
 * [등록 경로] Intent.Cart.SpillCleanup + Context.ItemUID 수신
 *   → 블랙보드(Stat)에 UID를 누적 기록
 *
 * [정리 경로] 일반 Interact Intent 수신 (부모 HoldingStampLogicModuleBase 처리)
 *   → 연속 홀딩 진행
 *   → 완료 시 블랙보드에 등록된 Spilled 아이템 UID 전부 파괴
 *   → 마커 자신도 파괴
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced,
       meta = (DisplayName = "Logic : Holding Spill Cleanup"))
class MOVEREXAMPLETEST_API ULogic_Holding_SpillCleanup : public UHoldingStampLogicModuleBase
{
    GENERATED_BODY()

public:
    ULogic_Holding_SpillCleanup();

    /** UID 등록 경로를 처리하고 나머지는 부모(Stamp)에 위임 */
    virtual bool PreInteractCheck(const FInteractionContext& Context) override;

    /** 블랙보드에 쏟인 아이템 UID를 축적할 때 사용하는 태그. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|SpillCleanup")
    FGameplayTag SpilledItemsStatTag;

protected:
    /** 홀딩 완료 시 Spilled 아이템을 전부 파괴하고 마커를 제거합니다. */
    virtual void OnStampCompleted_Implementation(const FInteractionContext& Context) override;
};
