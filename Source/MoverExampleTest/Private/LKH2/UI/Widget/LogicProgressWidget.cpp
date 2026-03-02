// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/UI/Widget/LogicProgressWidget.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"

void ULogicProgressWidget::InitializeProgressWidget(UInteractablePropertyComponent* InPropertyComp)
{
	TargetPropertyComp = InPropertyComp;
}

void ULogicProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ProgressBar)
	{
		ProgressBar->SetPercent(GetProgressRatio());
	}
}

float ULogicProgressWidget::GetProgressRatio() const
{
	if (!TargetPropertyComp.IsValid())
	{
		return 0.0f;
	}

	AActor* OwnerActor = TargetPropertyComp->GetOwner();
	if (!OwnerActor)
	{
		return 0.0f;
	}

	// 컨텍스트 인터페이스를 구현했는지 확인
	ILogicContextInterface* ContextInterface = Cast<ILogicContextInterface>(OwnerActor);
	if (!ContextInterface)
	{
		return 0.0f;
	}

	float CurrentRatio = 0.0f;

	// 1. Step 기반 진행 (CurrentStepTag가 할당되어 있다면 우선 처리)
	if (CurrentStepTag.IsValid() && MaxStepTag.IsValid())
	{
		// Blackboard 대신 InteractablePropertyComponent의 Replicated 필드를 직접 읽습니다.
		// 이 값은 서버에서 SetRepStepValues()로 설정되어 클라이언트에 복제됩니다.
		float CurrentStep = TargetPropertyComp.IsValid() ? TargetPropertyComp->GetRepCurrentStep() : 0.0f;
		float MaxStep     = TargetPropertyComp.IsValid() ? TargetPropertyComp->GetRepMaxStep()     : 0.0f;

		UE_LOG(LogTemp, Verbose, TEXT("[GetProgressRatio][Step] Owner=%s, CurrentStep=%f, MaxStep=%f"),
			OwnerActor ? *OwnerActor->GetName() : TEXT("?"), CurrentStep, MaxStep);

		if (MaxStep > 0.0f)
		{
			CurrentRatio = FMath::Clamp(CurrentStep / MaxStep, 0.0f, 1.0f);
		}
	}

	// 2. Time 기반 진행 (기존 연속 방식)
	else if (StartTimeTag.IsValid() && EndTimeTag.IsValid())
	{
		float StartTime = 0.0f;
		float EndTime = 0.0f;

		if (const FItemStatValue* StartStat = ContextInterface->FindStat(StartTimeTag))
		{
			if (StartStat->Type == EItemStatType::Float) StartTime = StartStat->FloatValue;
			else if (StartStat->Type == EItemStatType::Int) StartTime = StartStat->IntValue;
		}

		if (const FItemStatValue* EndStat = ContextInterface->FindStat(EndTimeTag))
		{
			if (EndStat->Type == EItemStatType::Float) EndTime = EndStat->FloatValue;
			else if (EndStat->Type == EItemStatType::Int) EndTime = EndStat->IntValue;
		}

		if (EndTime > StartTime && StartTime > 0.0f)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				float CurrentTime = World->GetTimeSeconds();
				CurrentRatio = FMath::Clamp((CurrentTime - StartTime) / (EndTime - StartTime), 0.0f, 1.0f);
			}
		}
	}

	if (bIsLocked)
	{
		return LockedProgress;
	}

	return CurrentRatio;
}

void ULogicProgressWidget::LockProgress()
{
	if (!bIsLocked)
	{
		LockedProgress = GetProgressRatio();
		bIsLocked = true;
	}
}

void ULogicProgressWidget::UnlockProgress()
{
	bIsLocked = false;
}

void ULogicProgressWidget::ResetProgress()
{
	bIsLocked = false;
	LockedProgress = 0.0f;
	StartTimeTag = FGameplayTag::EmptyTag;
	EndTimeTag = FGameplayTag::EmptyTag;
	CurrentStepTag = FGameplayTag::EmptyTag;
	MaxStepTag = FGameplayTag::EmptyTag;
}


