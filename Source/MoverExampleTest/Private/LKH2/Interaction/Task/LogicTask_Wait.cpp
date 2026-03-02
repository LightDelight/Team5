#include "LKH2/Interaction/Task/LogicTask_Wait.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"

ULogicTask_Wait::ULogicTask_Wait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ULogicTask_Wait* ULogicTask_Wait::Wait(ULogicModuleBase* OwningModule, float WaitTime, const FInteractionContext& Context)
{
	ULogicTask_Wait* MyTask = NewObject<ULogicTask_Wait>();
	MyTask->OwningLogicModule = OwningModule;
	MyTask->RequiredTime = WaitTime;
	MyTask->CachedContext = Context;
	return MyTask;
}

void ULogicTask_Wait::ReadyForActivation()
{
	Super::ReadyForActivation();
	
	if (RequiredTime <= 0.0f)
	{
		OnCompleted.Broadcast(this);
		EndTask();
	}
}

void ULogicTask_Wait::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	// Super::TickTask에서 이미 ElapsedTime을 증가시킴
	if (ElapsedTime >= RequiredTime)
	{
		OnCompleted.Broadcast(this);
		EndTask();
	}
}
