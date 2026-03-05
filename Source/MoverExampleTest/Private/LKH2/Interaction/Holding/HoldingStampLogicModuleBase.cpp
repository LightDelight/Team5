// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Holding/HoldingStampLogicModuleBase.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Base/LogicTaskBase.h"
#include "LKH2/Interaction/Task/LogicTask_Wait.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "LKH2/LKH2GameplayTags.h"
#include "Engine/World.h"

UHoldingStampLogicModuleBase::UHoldingStampLogicModuleBase()
{
    StartIntentTag  = FGameplayTag::RequestGameplayTag(TEXT("Interactor.Intent.Interact"));
    CancelIntentTag = FGameplayTag::RequestGameplayTag(TEXT("Interactor.Intent.Release"));
}

// ─────────────────────────────────────────
// 헬퍼: InteractorPropertyComponent 조회
// ─────────────────────────────────────────
static UInteractorPropertyComponent* FindInteractorProp(const FInteractionContext& Context)
{
    if (!Context.Interactor) return nullptr;
    return Context.Interactor->FindComponentByClass<UInteractorPropertyComponent>();
}

// ─────────────────────────────────────────
// 헬퍼: PropertyComponent 조회 (TargetActor 기준)
// ─────────────────────────────────────────
static UInteractablePropertyComponent* FindInteractableProp(const FInteractionContext& Context)
{
    if (!Context.TargetActor) return nullptr;
    return Context.TargetActor->FindComponentByClass<UInteractablePropertyComponent>();
}

// ─────────────────────────────────────────
// PreInteractCheck
// ─────────────────────────────────────────
bool UHoldingStampLogicModuleBase::PreInteractCheck(const FInteractionContext& Context)
{
    ULogicContextComponent* ContextComp = GetContextComponent(Context);
    if (!ContextComp) return false;

    // 취소 Intent: 진행 중인 태스크가 있을 때만 통과
    if (CancelIntentTag.IsValid() && Context.InteractionTag == CancelIntentTag)
    {
        return ContextComp->GetTask(TaskTag) != nullptr;
    }

    // 시작 Intent
    if (StartIntentTag.IsValid() && Context.InteractionTag == StartIntentTag)
    {
        // 이미 진행 중이면 중복 시작 방지
        if (ContextComp->GetTask(TaskTag) != nullptr) return false;

        // 거리 검사
        if (MaxInteractionDistance > 0.f && Context.Interactor && Context.TargetActor)
        {
            const float Dist = FVector::Dist(
                Context.Interactor->GetActorLocation(),
                Context.TargetActor->GetActorLocation());
            if (Dist > MaxInteractionDistance)
            {
                UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] 거리 초과 (%.0f > %.0f) → 시작 거부"),
                    Dist, MaxInteractionDistance);
                return false;
            }
        }
        return true;
    }

    return false;
}

// ─────────────────────────────────────────
// PerformInteraction
// ─────────────────────────────────────────
bool UHoldingStampLogicModuleBase::PerformInteraction(const FInteractionContext& Context)
{
    if (CancelIntentTag.IsValid() && Context.InteractionTag == CancelIntentTag)
    {
        CancelStamp(Context);
        return true;
    }

    if (StartIntentTag.IsValid() && Context.InteractionTag == StartIntentTag)
    {
        StartStamp(Context);
        return true;
    }

    return false;
}

// ─────────────────────────────────────────
// StartStamp
// ─────────────────────────────────────────
void UHoldingStampLogicModuleBase::StartStamp(const FInteractionContext& Context)
{
    ULogicContextComponent* ContextComp = GetContextComponent(Context);
    if (!ContextComp) return;

    // 저장된 진행도 복구 (이어하기)
    float SavedProgress = 0.f;
    if (bSaveProgressOnCancel && ProgressStatTag.IsValid())
    {
        if (const FItemStatValue* Saved = ContextComp->FindStat(ProgressStatTag))
        {
            SavedProgress = FMath::Clamp(Saved->FloatValue, 0.f, HoldDuration);
        }
    }

    const float Remaining = FMath::Max(0.1f, HoldDuration - SavedProgress);

    UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] 시작: 잔여%.2f초 (총%.2f초, 저장진행도%.2f)"),
        Remaining, HoldDuration, SavedProgress);

    // UI 시작 (이전 동결/타이머 잔여 상태 먼저 초기화 후 새 타이머 시작)
    if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
    {
        if (UInteractablePropertyComponent* PropComp = FindInteractableProp(Context))
        {
            // 이전 타이머 상태 초기화 (다른 플레이어가 남긴 잔여 StartTime/EndTime 제거)
            IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag);

            // 이전 FreezeHoldingProgress가 남긴 스텝 UI 초기화
            if (bMaintainUIOnCancel && FreezeCurrentStepTag.IsValid())
            {
                IM->ClearStepProgress(PropComp, FreezeCurrentStepTag, FreezeMaxStepTag);
            }

            // 새 타이머 UI 시작 (SavedProgress 위치부터)
            IM->StartHoldingProgress(PropComp, StartTimeTag, EndTimeTag,
                HoldDuration, FGameplayTag{}, FGuid{}, SavedProgress);
        }
    }

    // 몽타주 재생 (ActionTag 설정)
    if (ActionTag.IsValid())
    {
        if (UInteractorPropertyComponent* InteractorProp = FindInteractorProp(Context))
        {
            InteractorProp->SetActionTag(ActionTag);
            UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] ActionTag 설정: %s"), *ActionTag.ToString());
        }
    }

    // WaitTask 생성
    ULogicTask_Wait* WaitTask = ULogicTask_Wait::Wait(this, Remaining, Context, ContextComp);
    if (!WaitTask)
    {
        UE_LOG(LogTemp, Error, TEXT("[HoldingStamp] WaitTask 생성 실패"));
        return;
    }

    WaitTask->OnCompleted.AddDynamic(this, &UHoldingStampLogicModuleBase::HandleTaskCompleted);
    WaitTask->OnCanceled.AddDynamic(this,  &UHoldingStampLogicModuleBase::HandleTaskCanceled);
    ContextComp->SetTask(TaskTag, WaitTask);

    // 이동 차단
    if (UInteractorComponent* InteractorComp =
            Cast<UInteractorComponent>(Context.InteractorComp))
    {
        InteractorComp->SetIsWorking(true, Context.TargetActor, CancelIntentTag);
    }

    OnStampStarted(Context);
    WaitTask->ReadyForActivation();
}

// ─────────────────────────────────────────
// CancelStamp
// ─────────────────────────────────────────
void UHoldingStampLogicModuleBase::CancelStamp(const FInteractionContext& Context)
{
    ULogicContextComponent* ContextComp = GetContextComponent(Context);
    if (!ContextComp) return;

    ULogicTaskBase* ActiveTask = ContextComp->GetTask(TaskTag);
    if (!ActiveTask) return;

    // 진행도 저장 (이어하기용)
    if (bSaveProgressOnCancel && ProgressStatTag.IsValid())
    {
        // GetProgressRatio = ElapsedTime / WaitTime (LogicTask_Wait 기준)
        float SavedBefore = 0.f;
        if (const FItemStatValue* Prev = ContextComp->FindStat(ProgressStatTag))
            SavedBefore = Prev->FloatValue;

        const float ThisSession  = ActiveTask->GetProgressRatio() * (HoldDuration - SavedBefore);
        const float TotalElapsed = FMath::Clamp(SavedBefore + ThisSession, 0.f, HoldDuration);

        ContextComp->SetStat(ProgressStatTag, FItemStatValue::MakeFloat(TotalElapsed));

        UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] 진행도 저장: %.2f / %.2f"),
            TotalElapsed, HoldDuration);
    }
    else if (!bSaveProgressOnCancel && ProgressStatTag.IsValid())
    {
        ContextComp->SetStat(ProgressStatTag, FItemStatValue::MakeFloat(0.f));
    }

    // 몽타주 중지 (ActionTag 해제)
    if (ActionTag.IsValid())
    {
        if (UInteractorPropertyComponent* InteractorProp = FindInteractorProp(Context))
        {
            InteractorProp->ClearActionTag();
            UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] ActionTag 해제"));
        }
    }

    // UI 처리: 유지(Freeze) vs 제거(Clear)
    if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
    {
        if (UInteractablePropertyComponent* PropComp = FindInteractableProp(Context))
        {
            if (bMaintainUIOnCancel && FreezeCurrentStepTag.IsValid() && FreezeMaxStepTag.IsValid())
            {
                // 타이머 UI → 고정 스텝 UI로 변환 (위젯 유지)
                IM->FreezeHoldingProgress(PropComp, StartTimeTag, EndTimeTag,
                    FreezeCurrentStepTag, FreezeMaxStepTag);
                UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] 진행도 UI 동결 (FreezeHoldingProgress)"));
            }
            else
            {
                IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag);
            }
        }
    }

    ActiveTask->CancelTask();
    ContextComp->ClearTask(TaskTag);

    // 이동 차단 해제
    if (UInteractorComponent* InteractorComp =
            Cast<UInteractorComponent>(Context.InteractorComp))
    {
        InteractorComp->SetIsWorking(false);
    }

    OnStampCanceled(Context);
}

// ─────────────────────────────────────────
// HandleTaskCompleted
// ─────────────────────────────────────────
void UHoldingStampLogicModuleBase::HandleTaskCompleted(ULogicTaskBase* TaskInstance)
{
    if (!TaskInstance) return;

    const FInteractionContext& Context = TaskInstance->GetCachedContext();
    ULogicContextComponent* ContextComp = GetContextComponent(Context);
    if (!ContextComp) return;

    ContextComp->ClearTask(TaskTag);

    // 진행도 초기화 (완료됐으므로)
    if (ProgressStatTag.IsValid())
    {
        ContextComp->SetStat(ProgressStatTag, FItemStatValue::MakeFloat(0.f));
    }

    // 몽타주 중지
    if (ActionTag.IsValid())
    {
        if (UInteractorPropertyComponent* InteractorProp = FindInteractorProp(Context))
        {
            InteractorProp->ClearActionTag();
        }
    }

    // UI 제거 (완료 시에는 항상 제거)
    if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
    {
        if (UInteractablePropertyComponent* PropComp = FindInteractableProp(Context))
        {
            IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag);
        }
    }

    // 이동 차단 해제
    if (UInteractorComponent* InteractorComp =
            Cast<UInteractorComponent>(Context.InteractorComp))
    {
        InteractorComp->SetIsWorking(false);
    }

    UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] 완료: %s"),
        Context.TargetActor ? *Context.TargetActor->GetName() : TEXT("?"));

    OnStampCompleted(Context);
}

// ─────────────────────────────────────────
// HandleTaskCanceled (Task 측에서 강제 취소된 경우)
// ─────────────────────────────────────────
void UHoldingStampLogicModuleBase::HandleTaskCanceled(ULogicTaskBase* TaskInstance)
{
    if (!TaskInstance) return;

    const FInteractionContext& Context = TaskInstance->GetCachedContext();
    ULogicContextComponent* ContextComp = GetContextComponent(Context);
    if (ContextComp)
    {
        ContextComp->ClearTask(TaskTag);
    }

    if (ActionTag.IsValid())
    {
        if (UInteractorPropertyComponent* InteractorProp = FindInteractorProp(Context))
        {
            InteractorProp->ClearActionTag();
        }
    }

    if (UInteractorComponent* InteractorComp =
            Cast<UInteractorComponent>(Context.InteractorComp))
    {
        InteractorComp->SetIsWorking(false);
    }

    OnStampCanceled(Context);
}

// ─────────────────────────────────────────
// Events (기본 구현)
// ─────────────────────────────────────────
void UHoldingStampLogicModuleBase::OnStampStarted_Implementation(const FInteractionContext& Context)
{
    UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] Stamp Started"));
}

void UHoldingStampLogicModuleBase::OnStampCompleted_Implementation(const FInteractionContext& Context)
{
    UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] Stamp Completed!"));
}

void UHoldingStampLogicModuleBase::OnStampCanceled_Implementation(const FInteractionContext& Context)
{
    UE_LOG(LogTemp, Log, TEXT("[HoldingStamp] Stamp Canceled"));
}

// ─────────────────────────────────────────
// GetContextComponent
// ─────────────────────────────────────────
ULogicContextComponent* UHoldingStampLogicModuleBase::GetContextComponent(const FInteractionContext& Context) const
{
    if (Context.TargetActor)
    {
        return Context.TargetActor->FindComponentByClass<ULogicContextComponent>();
    }
    return nullptr;
}
