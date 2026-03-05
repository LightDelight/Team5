// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/UI/Widget/LogicProgressWidget.h"
#include "GameFramework/GameStateBase.h"
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
	const FProgressUIState& UIState = TargetPropertyComp->GetUIState();

	// 1. Step 기반 진행 (고정 수치)
	if (UIState.Mode == EProgressDisplayMode::Step)
	{
		float CurrentStep = UIState.CurrentStep;
		float MaxStep     = UIState.MaxStep;

		if (MaxStep > 0.0f)
		{
			CurrentRatio = FMath::Clamp(CurrentStep / MaxStep, 0.0f, 1.0f);
		}
	}
	// 2. Time 기반 진행 (연속 방식)
	else if (UIState.Mode == EProgressDisplayMode::Timer)
	{
		float StartTime = 0.0f;
		float EndTime = 0.0f;

		if (const FItemStatValue* StartStat = ContextInterface->FindStat(UIState.StartTimeTag))
		{
			if (StartStat->Type == EItemStatType::Float) StartTime = StartStat->FloatValue;
		}

		if (const FItemStatValue* EndStat = ContextInterface->FindStat(UIState.EndTimeTag))
		{
			if (EndStat->Type == EItemStatType::Float) EndTime = EndStat->FloatValue;
		}

		if (EndTime > StartTime && StartTime > 0.0f)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				float CurrentTime = 0.0f;
				if (AGameStateBase* GS = World->GetGameState())
				{
					CurrentTime = GS->GetServerWorldTimeSeconds();
				}
				else
				{
					CurrentTime = World->GetTimeSeconds();
				}

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


