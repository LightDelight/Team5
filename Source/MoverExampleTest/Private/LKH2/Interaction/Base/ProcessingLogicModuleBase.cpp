#include "LKH2/Interaction/Base/ProcessingLogicModuleBase.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Task/LogicTask_Wait.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "Engine/World.h"

UProcessingLogicModuleBase::UProcessingLogicModuleBase()
{
}

bool UProcessingLogicModuleBase::CanStartProcessing() const
{
	if (!OwnerActor) return false;

	if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
	{
		int32 ItemCount = 0;
		for (auto& Pair : PropComp->StoredItems)
		{
			if (Pair.Value != nullptr) ItemCount++;
		}
		return ItemCount >= RequiredItemCount;
	}

	return false;
}

void UProcessingLogicModuleBase::StartProcessing(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp || !OwnerActor) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] StartProcessing on Actor: %s"), *GetName(), *OwnerActor->GetName());

	// 이미 진행 중인 태스크가 있다면 중단 후 새로 시작 (안정성)
	if (ContextComp->GetTask(ProcessingTaskTag))
	{
		StopProcessing(Context);
	}

	// UI 표시 (Timestamp 방식)
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
		{
			IM->StartHoldingProgress(PropComp, StartTimeTag, EndTimeTag, ProcessingDuration);
		}
	}

	// Wait 태스크 생성 및 컨텍스트에 저장 (Stateless 관리 핵심)
	ULogicTask_Wait* WaitTask = ULogicTask_Wait::Wait(this, ProcessingDuration, Context);
	if (WaitTask)
	{
		ContextComp->SetTask(ProcessingTaskTag, WaitTask);
		WaitTask->ReadyForActivation();
	}
}

void UProcessingLogicModuleBase::StopProcessing(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] StopProcessing on Actor: %s"), *GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));

	// UI 해제
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
		{
			IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag);
		}
	}

	// 컨텍스트에서 태스크를 찾아 취소 및 제거
	if (ULogicTaskBase* ActiveTask = ContextComp->GetTask(ProcessingTaskTag))
	{
		ActiveTask->CancelTask();
		ContextComp->ClearTask(ProcessingTaskTag);
	}
}

bool UProcessingLogicModuleBase::ExecuteInteraction(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp) return false;

	// 1. 시작 의도 확인
	if (Context.InteractionTag.MatchesTag(StartIntentTag))
	{
		if (CanStartProcessing())
		{
			// 컨텍스트에 태스크가 없는 경우에만 시작
			if (ContextComp->GetTask(ProcessingTaskTag) == nullptr)
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] ExecuteInteraction - Matching StartIntentTag: %s"), *GetName(), *StartIntentTag.ToString());
				StartProcessing(Context);
			}
			return true;
		}
	}
	// 2. 중단 의도 확인
	else if (Context.InteractionTag.MatchesTag(StopIntentTag))
	{
		if (ContextComp->GetTask(ProcessingTaskTag) != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] ExecuteInteraction - Matching StopIntentTag: %s"), *GetName(), *StopIntentTag.ToString());
			StopProcessing(Context);
			return true;
		}
	}

	return Super::ExecuteInteraction(Context);
}

void UProcessingLogicModuleBase::OnProcessingCompleted_Implementation(const FInteractionContext& Context)
{
	// 추후 파생 클래스에서 아이템 변환 로직 등을 작성
}

void UProcessingLogicModuleBase::HandleTaskCompleted(ULogicTaskBase* TaskInstance)
{
	if (!TaskInstance) return;

	const FInteractionContext& Context = TaskInstance->GetCachedContext();
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp) return;

	// 내가 관리하는 프로세싱 태스크가 맞는지 확인
	if (TaskInstance == ContextComp->GetTask(ProcessingTaskTag))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] HandleTaskCompleted - Processing Task Finished on Actor: %s"), *GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));

		// 태스크 제거
		ContextComp->ClearTask(ProcessingTaskTag);

		// UI 정지 (성공)
		if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
		{
			if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
			{
				IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag);
			}
		}

		OnProcessingCompleted(Context);
	}
}

void UProcessingLogicModuleBase::HandleTaskCanceled(ULogicTaskBase* TaskInstance)
{
	if (!TaskInstance) return;

	const FInteractionContext& Context = TaskInstance->GetCachedContext();
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (ContextComp)
	{
		if (TaskInstance == ContextComp->GetTask(ProcessingTaskTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] HandleTaskCanceled - Processing Task Canceled on Actor: %s"), *GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));
			ContextComp->ClearTask(ProcessingTaskTag);
		}
	}
}
