// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Base/LogicTaskBase.h"

#include "LKH2/Interaction/Base/LogicModuleBase.h"

ULogicTaskBase::ULogicTaskBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsTaskActive = false;
}

ULogicTaskBase* ULogicTaskBase::CreateLogicTask(ULogicModuleBase* OwningModule, TSubclassOf<ULogicTaskBase> TaskClass, const FInteractionContext& Context)
{
	if (!OwningModule || !TaskClass)
	{
		return nullptr;
	}

	ULogicTaskBase* NewTask = NewObject<ULogicTaskBase>(OwningModule, TaskClass);
	NewTask->OwningLogicModule = OwningModule;
	NewTask->CachedContext = Context;
	return NewTask;
}

float ULogicTaskBase::GetProgressRatio() const
{
	if (RequiredTime <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(ElapsedTime / RequiredTime, 0.0f, 1.0f);
}

void ULogicTaskBase::ReadyForActivation()
{
	bIsTaskActive = true;
	ElapsedTime = 0.0f;

	// 주기적으로 TickTask를 호출하는 타이머 등록
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimerHandle,
			[this]() { TickTask(TaskTickInterval); },
			TaskTickInterval,
			true
		);
	}
}

void ULogicTaskBase::EndTask()
{
	if (!bIsTaskActive) return;

	bIsTaskActive = false;
	OnTaskEnded();

	// 비동기 액션 노드 종료 및 GC 허용
	SetReadyToDestroy();
}

void ULogicTaskBase::TickTask(float DeltaTime)
{
	if (!bIsTaskActive) return;

	ElapsedTime += DeltaTime;
	OnTick.Broadcast(DeltaTime);
}

void ULogicTaskBase::OnTaskEnded()
{
	// 등록된 타이머 해제
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimerHandle);
	}
}

void ULogicTaskBase::CancelTask()
{
	if (!bIsTaskActive) return;

	// OnCanceled를 먼저 발동시켜 Module의 HandleTaskCanceled가 호출되도록 보장
	OnCanceled.Broadcast(this);

	// EndTask는 bIsTaskActive 체크 후 진행 — OnCanceled 핸들러에서
	// 이미 EndTask를 호출했을 수 있으므로 내부에서 중복 방지됨
	EndTask();
}
