#include "LKH2/Interaction/Base/ProcessingLogicModuleBase.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Task/LogicTask_Wait.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/LKH2GameplayTags.h"

UProcessingLogicModuleBase::UProcessingLogicModuleBase()
{
	const FLKH2GameplayTags& Tags = FLKH2GameplayTags::Get();

	// 기본 태그 설정: 에디터에서 비워둘 경우 이 태그들이 사용됩니다.
	CurrentStepTag = Tags.Stat_Common_CurrentProgress;
	MaxStepTag = Tags.Stat_Common_MaxProgress;
	StartTimeTag = Tags.Time_Common_StartTime;
	EndTimeTag = Tags.Time_Common_EndTime;

	StartIntentTag = Tags.Intent_Workstation_ItemAdd;
	StopIntentTag = Tags.Intent_Workstation_ItemRemove;
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

	// 1. 가공 대상 및 진행도 확인 (Container 대응 포함)
	float InitialProgress = 0.0f;
	FGuid TargetItemUID = Context.ItemUID;
	FGuid UISyncUID = Context.ItemUID; // UI 동기화에 사용할 UID (컨테이너인 경우 컨테이너 UID)
	
	UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>();
	FGameplayTag TargetLookupTag = UISlotTag.IsValid() ? UISlotTag : (Context.SlotTag.IsValid() ? Context.SlotTag : FGameplayTag::RequestGameplayTag(TEXT("Slot.Main")));

	if (PropComp)
	{
		AItemBase* SlotItem = PropComp->GetStoredItem(TargetLookupTag);
		AItemBase* TargetItem = GetTargetItemDeep(Context, PropComp, TargetLookupTag);

		if (SlotItem)
		{
			UISyncUID = SlotItem->GetInstanceId();
			
			// 겉표면 아이템(컨테이너 등)에서 진행도 조회
			if (ILogicContextInterface* UIContext = Cast<ILogicContextInterface>(SlotItem))
			{
				if (const FItemStatValue* SavedProgress = UIContext->FindStat(CurrentStepTag))
				{
					if (SavedProgress->FloatValue > 0)
					{
						InitialProgress = SavedProgress->FloatValue;
					}
				}
			}
		}

		if (TargetItem)
		{
			if (TargetItem->GetInstanceId().IsValid())
			{
				TargetItemUID = TargetItem->GetInstanceId();
			}
			UE_LOG(LogTemp, Log, TEXT("[%s] Targeting Item for Processing: %s (UID: %s), UI Linked to: %s (UID: %s), InitialProgress: %f"), 
				*GetName(), *TargetItem->GetName(), *TargetItemUID.ToString(), 
				SlotItem ? *SlotItem->GetName() : TEXT("None"), *UISyncUID.ToString(), InitialProgress);
		}
	}

	// 2. UI 표시 (Timestamp 방식)
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		if (PropComp)
		{
			// 시작 전 기존에 남아있을 수 있는 Step UI(동결된 UI) 제거
			IM->ClearStepProgress(PropComp, CurrentStepTag, MaxStepTag, TargetLookupTag, UISyncUID);

			// 타이머 기반 UI 시작
			IM->StartHoldingProgress(PropComp, StartTimeTag, EndTimeTag, ProcessingDuration, TargetLookupTag, UISyncUID, InitialProgress);
		}
	}

	// Wait 태스크 생성 및 컨텍스트에 저장 (Stateless 관리 핵심)
	float RemainingDuration = FMath::Max(0.1f, ProcessingDuration - InitialProgress);
	
	// 가공 대상 UID로 갱신된 컨텍스트 생성
	FInteractionContext UpdatedContext = Context;
	UpdatedContext.ItemUID = TargetItemUID;

	ULogicTask_Wait* WaitTask = ULogicTask_Wait::Wait(this, RemainingDuration, UpdatedContext, ContextComp);
	if (WaitTask)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] StartProcessing - Task Created for ItemUID: %s, Duration: %f"), *GetName(), *TargetItemUID.ToString(), ProcessingDuration);

		// 델리게이트 바인딩: 태스크 완료/취소 시 모듈의 핸들러가 호출되도록 함
		WaitTask->OnCompleted.AddDynamic(this, &UProcessingLogicModuleBase::HandleTaskCompleted);
		WaitTask->OnCanceled.AddDynamic(this, &UProcessingLogicModuleBase::HandleTaskCanceled);

		ContextComp->SetTask(ProcessingTaskTag, WaitTask);
		WaitTask->ReadyForActivation();
		UE_LOG(LogTemp, Log, TEXT("[%s] StartProcessing - Task Activated and Stored in Context with Tag: %s"), *GetName(), *ProcessingTaskTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] StartProcessing - Failed to create WaitTask!"), *GetName());
	}
}

void UProcessingLogicModuleBase::StopProcessing(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComp = Cast<ULogicContextComponent>(Context.ContextComp);
	if (!ContextComp) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] StopProcessing on Actor: %s"), *GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));

	// UI 처리: 설정에 따라 취소 시에도 유지하거나 동결(Freeze)
	if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
	{
		if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
		{
			FGameplayTag TargetLookupTag = UISlotTag.IsValid() ? UISlotTag : (Context.SlotTag.IsValid() ? Context.SlotTag : FGameplayTag::RequestGameplayTag(TEXT("Slot.Main")));
			
			// UI가 붙어있는 실제 겉표면 아이템 (컨테이너 등)
			AItemBase* SlotItem = PropComp->GetStoredItem(TargetLookupTag);
			FGuid UISyncUID = Context.ItemUID;
			if (SlotItem && SlotItem->GetInstanceId().IsValid())
			{
				UISyncUID = SlotItem->GetInstanceId();
			}

			if (bMaintainUIOnCancel)
			{
				// 진행도를 계산하여 Step UI로 동결
				IM->FreezeHoldingProgress(PropComp, StartTimeTag, EndTimeTag, CurrentStepTag, MaxStepTag, TargetLookupTag, UISyncUID);
			}
			else
			{
				// UI 완전히 제거
				IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag, TargetLookupTag, UISyncUID);
			}
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
		UE_LOG(LogTemp, Log, TEXT("[%s] ExecuteInteraction - Matching StartIntentTag: %s"), *GetName(), *StartIntentTag.ToString());
		if (CanStartProcessing())
		{
			// 컨텍스트에 태스크가 없는 경우에만 시작
			if (ContextComp->GetTask(ProcessingTaskTag) == nullptr)
			{
				StartProcessing(Context);
			}
			return true;
		}
	}
	// 2. 중단 의도 확인
	else if (Context.InteractionTag.MatchesTag(StopIntentTag))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] ExecuteInteraction - Matching StopIntentTag: %s"), *GetName(), *StopIntentTag.ToString());
		if (ContextComp->GetTask(ProcessingTaskTag) != nullptr)
		{
			StopProcessing(Context);
			return true;
		}
	}
	else
	{
		// 기타 의도 로깅 (디버그용)
		UE_LOG(LogTemp, Verbose, TEXT("[%s] ExecuteInteraction - Unhandled Intent: %s"), *GetName(), *Context.InteractionTag.ToString());
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
		UE_LOG(LogTemp, Log, TEXT("[%s] HandleTaskCompleted - VERIFIED: Task matches active processing task on Actor: %s"), *GetName(), OwnerActor ? *OwnerActor->GetName() : TEXT("None"));

		// 태스크 제거
		ContextComp->ClearTask(ProcessingTaskTag);

		// UI 정지 (성공 시에는 설정을 무시하고 반드시 제거)
		if (UInteractionManager* IM = GetWorld()->GetSubsystem<UInteractionManager>())
		{
			if (UInteractablePropertyComponent* PropComp = OwnerActor->FindComponentByClass<UInteractablePropertyComponent>())
			{
				FGameplayTag TargetLookupTag = UISlotTag.IsValid() ? UISlotTag : (Context.SlotTag.IsValid() ? Context.SlotTag : FGameplayTag::RequestGameplayTag(TEXT("Slot.Main")));
				
				// UI가 연동된 실제 겉표면 아이템 (컨테이너 등)
				AItemBase* SlotItem = PropComp->GetStoredItem(TargetLookupTag);
				FGuid UISyncUID = Context.ItemUID;
				if (SlotItem) UISyncUID = SlotItem->GetInstanceId();

				IM->ClearHoldingProgress(PropComp, StartTimeTag, EndTimeTag, TargetLookupTag, UISyncUID);
				IM->ClearStepProgress(PropComp, CurrentStepTag, MaxStepTag, TargetLookupTag, UISyncUID);
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

			// UI 동결/제거 처리를 위해 StopProcessing 호출
			StopProcessing(Context);
		}
	}
}

AItemBase* UProcessingLogicModuleBase::GetTargetItemDeep(const FInteractionContext& Context, UInteractablePropertyComponent* PropComp, FGameplayTag SlotTag) const
{
	if (!PropComp) return nullptr;

	AItemBase* SlotItem = PropComp->GetStoredItem(SlotTag);
	if (!SlotItem) return nullptr;

	// 컨테이너 타입인지 태그로 확인
	if (const UItemData* ItemData = SlotItem->GetItemData())
	{
		if (ContainerTypeTag.IsValid() && ItemData->ItemTag.MatchesTag(ContainerTypeTag))
		{
			// 컨테이너라면 내부의 첫 번째 아이템을 찾아 반환
			if (UInteractablePropertyComponent* ItemProp = SlotItem->FindComponentByClass<UInteractablePropertyComponent>())
			{
				for (auto& Pair : ItemProp->StoredItems)
				{
					if (Pair.Value != nullptr)
					{
						return Pair.Value;
					}
				}
			}
		}
	}

	// 컨테이너가 아니거나 내부가 비었다면 슬롯 아이템 자체를 반환
	return SlotItem;
}
