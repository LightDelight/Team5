// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Task/LogicTask_PlayMontageAndWait.h"
#include "Engine/World.h"
#include "LKh2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"

ULogicTask_PlayMontageAndWait::ULogicTask_PlayMontageAndWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxStep = 5.0f;
	CurrentStep = 0.0f;
}

void ULogicTask_PlayMontageAndWait::ReadyForActivation()
{
	Super::ReadyForActivation();

	bool bHasInteractorProp = CachedContext.InteractorPropertyComp != nullptr;
	UE_LOG(LogTemp, Log, TEXT("[LogicTask_PlayMontageAndWait] ReadyForActivation 호용됨 - ActionTag: %s, bHasInteractorProp: %d"), *ActionTag.ToString(), bHasInteractorProp);

	if (!ActionTag.IsValid() || !bHasInteractorProp)
	{
		UE_LOG(LogTemp, Error, TEXT("[LogicTask_PlayMontageAndWait] 실패: ActionTag가 유효하지 않거나 InteractorPropertyComp가 NULL입니다."));
		EndTask();
		return;
	}

	UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(CachedContext.InteractorPropertyComp);
	if (InteractorProp)
	{
		UE_LOG(LogTemp, Log, TEXT("[LogicTask_PlayMontageAndWait] PropertyComponent에 SetActionTag(%s) 명령을 전달합니다."), *ActionTag.ToString());
		InteractorProp->SetActionTag(ActionTag);

		// 거리 모니터링 시작 (Task가 상태를 직접 관리)
		if (MaxInteractionDistance > 0.f && CachedContext.Interactor && CachedContext.TargetActor)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					DistanceCheckTimerHandle,
					this,
					&ULogicTask_PlayMontageAndWait::CheckDistance,
					0.1f,
					true);
			}
		}
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("[LogicTask_PlayMontageAndWait] 실패: InteractorPropertyComp 캐스팅 실패."));
	EndTask();
}

void ULogicTask_PlayMontageAndWait::AdvanceProgress()
{
	if (!bIsTaskActive) return;

	CurrentStep += 1.0f;
	
	UE_LOG(LogTemp, Log, TEXT("[LogicTask_PlayMontageAndWait] 진행도 증가! (%f / %f)"), CurrentStep, MaxStep);

	// 순수하게 진행도 변화만 알립니다. 
	// UI를 그릴지, 데이터를 저장할지는 이 태스크를 관리하는 Module(현장 관리자)이 결정합니다.
	OnProgressUpdated.Broadcast(this, CurrentStep, MaxStep);

	// 목표치 달성 시 알림 발송 (종료 여부는 상황에 따라 Module이 판단할 수 있으나, 
	// 기본적으로 몽타주 기반 태스크의 한 사이클이 끝났음을 의미하므로 알림)
	if (CurrentStep >= MaxStep)
	{
		OnCompleted.Broadcast(this);
		// 주의: 여기서 직접 EndTask()를 호출할지, Module이 호출하게 할지 선택 가능합니다.
		// 일단 기존 흐름 유지를 위해 스스로 종료합니다.
		EndTask();
	}
}

void ULogicTask_PlayMontageAndWait::EndTask()
{
	UE_LOG(LogTemp, Log, TEXT("[LogicTask_PlayMontageAndWait] EndTask 호용됨 (bIsTaskActive: %d)"), bIsTaskActive);

	// 거리 모니터링 정지
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DistanceCheckTimerHandle);
	}

	if (bIsTaskActive)
	{
		// 애니베이션 정지를 위해 태그 회수
		if (CachedContext.InteractorPropertyComp)
		{
			if (UInteractorPropertyComponent* InteractorProp = Cast<UInteractorPropertyComponent>(CachedContext.InteractorPropertyComp))
			{
				UE_LOG(LogTemp, Log, TEXT("[LogicTask_PlayMontageAndWait] PropertyComponent의 ActionTag를 Clear하여 몽타주를 중지합니다."));
				InteractorProp->ClearActionTag();
			}
		}
	}

	Super::EndTask();
}

void ULogicTask_PlayMontageAndWait::CheckDistance()
{
	if (!CachedContext.Interactor || !CachedContext.TargetActor) return;

	const float Dist = FVector::Dist(
		CachedContext.Interactor->GetActorLocation(),
		CachedContext.TargetActor->GetActorLocation());

	if (Dist > MaxInteractionDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LogicTask_PlayMontageAndWait] 거리 초과 (%.0f > %.0f), 홀딩 강제 취소"), Dist, MaxInteractionDistance);
		// CancelTask() → OnCanceled 발동 → HandleTaskCanceled → SetIsWorking(false)
		CancelTask();
	}
}
