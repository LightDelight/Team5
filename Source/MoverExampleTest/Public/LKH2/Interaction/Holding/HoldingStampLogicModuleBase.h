#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "HoldingStampLogicModuleBase.generated.h"

/**
 * 버튼을 누르고 있는 동안 진행도가 연속적으로 차오르는 홀딩 로직의 베이스 클래스.
 *
 * ─ HoldingStepLogicModuleBase와의 차이 ─
 *   Step  : 애니메이션 Notify마다 이산적으로 진행 (예: 5번 찍기)
 *   Stamp : 누르고 있는 시간만큼 연속적으로 진행 (예: 3초 홀드)
 *
 * ─ 상태 흐름 ─
 *   [Interact] → WaitTask 생성 & 타이머 UI 시작
 *   [Release/Cancel] → 태스크 취소, 진행도를 ContextComp에 저장 (이어하기 지원)
 *   [WaitTask 완료] → OnStampCompleted 호출
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API UHoldingStampLogicModuleBase : public ULogicModuleBase
{
    GENERATED_BODY()

public:
    UHoldingStampLogicModuleBase();

    virtual bool PreInteractCheck(const FInteractionContext& Context) override;
    virtual bool PerformInteraction(const FInteractionContext& Context) override;

    virtual void HandleTaskCompleted(class ULogicTaskBase* TaskInstance) override;
    virtual void HandleTaskCanceled(class ULogicTaskBase* TaskInstance) override;

    // ─── 공통 설정 ───

    /** 홀딩 시작 Intent 태그 (예: Interactor.Intent.Interact) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag StartIntentTag;

    /** 홀딩 취소 Intent 태그 (예: Interactor.Intent.Release) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag CancelIntentTag;

    /** Interact 시 InteractorPropertyComponent에 전달할 몽타주 Action 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag ActionTag;

    /** 태스크를 ContextComp에 보관할 때 사용할 식별 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag TaskTag;

    /** 진행도를 ContextComp 블랙보드에 저장할 때 사용할 스탯 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag ProgressStatTag;

    /** 타이머 UI 표시를 위한 시작 시간 스탯 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag StartTimeTag;

    /** 타이머 UI 표시를 위한 종료 시간 스탯 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag")
    FGameplayTag EndTimeTag;

    /** 완료까지 필요한 총 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Value",
              meta = (ClampMin = "0.1"))
    float HoldDuration = 3.0f;

    /** 최대 상호작용 거리 (cm, 0 이하면 비활성화) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Value")
    float MaxInteractionDistance = 300.0f;

    /**
     * 취소 시 진행도를 ContextComp에 유지할지 여부.
     * true  → 다음 Interact 시 이어하기 가능
     * false → 취소 시 진행도 초기화
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Value")
    bool bSaveProgressOnCancel = true;

    /**
     * true 시 취소 후에도 위젯이 사라지지 않고 글일신 진행도를 표시합니다.
     * FreezeHoldingProgress를 계산하치 위해 CurrentStepTag/MaxStepTag를 설정해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Value")
    bool bMaintainUIOnCancel = false;

    /** bMaintainUIOnCancel 사용 시 FreezeHoldingProgress에 전달할 현재 진행도 스탯 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag",
              meta = (EditCondition = "bMaintainUIOnCancel"))
    FGameplayTag FreezeCurrentStepTag;

    /** bMaintainUIOnCancel 사용 시 FreezeHoldingProgress에 전달할 최대 진행도 스탯 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Stamp|Tag",
              meta = (EditCondition = "bMaintainUIOnCancel"))
    FGameplayTag FreezeMaxStepTag;

protected:
    /** [Stamp Complete] 홀딩이 성공적으로 완료됐을 때 호출됩니다. */
    UFUNCTION(BlueprintNativeEvent, Category = "Logic|Stamp|Events")
    void OnStampCompleted(const FInteractionContext& Context);
    virtual void OnStampCompleted_Implementation(const FInteractionContext& Context);

    /** [Stamp Canceled] 도중 취소됐을 때 호출됩니다. */
    UFUNCTION(BlueprintNativeEvent, Category = "Logic|Stamp|Events")
    void OnStampCanceled(const FInteractionContext& Context);
    virtual void OnStampCanceled_Implementation(const FInteractionContext& Context);

    /** [Stamp Started] 홀딩이 시작될 때 호출됩니다. */
    UFUNCTION(BlueprintNativeEvent, Category = "Logic|Stamp|Events")
    void OnStampStarted(const FInteractionContext& Context);
    virtual void OnStampStarted_Implementation(const FInteractionContext& Context);

private:
    /** Context의 TargetActor에서 LogicContextComponent를 조회합니다. */
    class ULogicContextComponent* GetContextComponent(const FInteractionContext& Context) const;

    /** 태스크를 생성하고 타이머를 시작합니다. */
    void StartStamp(const FInteractionContext& Context);

    /** 태스크를 취소하고 진행도를 보존합니다. */
    void CancelStamp(const FInteractionContext& Context);
};
