#include "LKH2/Interaction/Holding/HoldingStepLogicModuleBase.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Base/LogicTaskBase.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LKH2/Interaction/Task/LogicTask_PlayMontageAndWait.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKh2/Interaction/Manager/ItemRecipeManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UHoldingStepLogicModuleBase::UHoldingStepLogicModuleBase()
{
	TaskClass = ULogicTask_PlayMontageAndWait::StaticClass();
}

bool UHoldingStepLogicModuleBase::PreInteractCheck(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComponent = GetContextComponent(Context);
	if (!ContextComponent) return false;

	// 1. 취소 의도 (Release, Cancel)
	if (CancelIntentTag.IsValid() && Context.InteractionTag == CancelIntentTag)
	{
		if (ContextComponent->GetTask(TaskTag) != nullptr) return true;
	}

	// 2. 시작 의도 (Hold/Press)
	if (Context.InteractionTag == RequiredIntentTag)
	{
		// 거리 사전 검사
		if (MaxInteractionDistance > 0.f && Context.Interactor && Context.TargetActor)
		{
			const float Dist = FVector::Dist(
				Context.Interactor->GetActorLocation(),
				Context.TargetActor->GetActorLocation());
			if (Dist > MaxInteractionDistance)
			{
				UE_LOG(LogTemp, Log, TEXT("[HoldingStepLogic] PreInteractCheck: 거리 초과 (%.0f > %.0f), 시작 거부"), Dist, MaxInteractionDistance);
				return false;
			}
		}

		// 아이템 및 레시피 유효성 검사
		if (UInteractablePropertyComponent* InteractableProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp))
		{
			AItemBase* StoredItem = InteractableProp->GetStoredItem(SlotTag);
			if (!StoredItem) return false;

			if (RecipeTag.IsValid())
			{
				if (UGameInstance* GameInstance = Context.Interactor->GetWorld()->GetGameInstance())
				{
					if (UItemRecipeManager* RecipeManager = GameInstance->GetSubsystem<UItemRecipeManager>())
					{
						if (UItemData* ItemData = StoredItem->GetItemData())
						{
							TArray<FGameplayTag> Inputs = { ItemData->ItemTag };
							if (RecipeManager->GetRecipeResultMulti(Inputs, RecipeTag).IsEmpty()) return false;
						}
					}
				}
			}
		}
		return true;
	}

	// 3. 진행 의도 (Notify)
	if (ProgressIntentTag.IsValid() && Context.InteractionTag == ProgressIntentTag)
	{
		if (ContextComponent->GetTask(TaskTag) != nullptr) return true;
	}

	return false;
}

bool UHoldingStepLogicModuleBase::PerformInteraction(const FInteractionContext& Context)
{
	ULogicContextComponent* ContextComponent = GetContextComponent(Context);
	if (!ContextComponent) return false;

	// [Holding 시작]
	if (Context.InteractionTag == RequiredIntentTag)
	{
		if (ULogicTaskBase* ExistingTask = ContextComponent->GetTask(TaskTag))
		{
			ExistingTask->EndTask();
			ContextComponent->ClearTask(TaskTag);
		}

		if (TaskClass)
		{
			ULogicTaskBase* NewTask = ULogicTaskBase::CreateLogicTask(this, TaskClass, Context);
			if (NewTask)
			{
				// 태스크 구체적 속성 주입 (가상 함수 호출)
				ConfigureTask(NewTask, Context);

				// 델리게이트 구독
				NewTask->OnCompleted.AddDynamic(this, &UHoldingStepLogicModuleBase::HandleTaskCompleted);
				NewTask->OnCanceled.AddDynamic(this, &UHoldingStepLogicModuleBase::HandleTaskCanceled);
				
				if (ULogicTask_PlayMontageAndWait* PlayTask = Cast<ULogicTask_PlayMontageAndWait>(NewTask))
				{
					PlayTask->OnProgressUpdated.AddDynamic(this, &UHoldingStepLogicModuleBase::HandleTaskProgressUpdated);
				}

				ContextComponent->SetTask(TaskTag, NewTask);

				// 작업 시작 플래그 설정 (이동 차단 + 타겟 변경 감지용 정보 전달)
				if (UInteractorComponent* InteractorComp = Cast<UInteractorComponent>(Context.InteractorComp))
				{
					InteractorComp->SetIsWorking(true, Context.TargetActor, CancelIntentTag);
				}

				// 시작 이벤트 호출
				OnHoldingStarted(Context);

				NewTask->ReadyForActivation();
				return true;
			}
		}
	}
	// [Holding 취소]
	else if (CancelIntentTag.IsValid() && Context.InteractionTag == CancelIntentTag)
	{
		if (ULogicTaskBase* ActiveTask = ContextComponent->GetTask(TaskTag))
		{
			// CancelTask()는 OnCanceled를 먼저 발동한 뒤 EndTask()를 호출합니다.
			// EndTask()를 직접 호출하면 OnCanceled가 발식되지 않아
			// HandleTaskCanceled → SetIsWorking(false)가 실행되지 않습니다.
			ActiveTask->CancelTask();
			return true;
		}
	}
	// [진행 단계 - Notify]
	else if (ProgressIntentTag.IsValid() && Context.InteractionTag == ProgressIntentTag)
	{
		if (ULogicTaskBase* ActiveTask = ContextComponent->GetTask(TaskTag))
		{
			if (ULogicTask_PlayMontageAndWait* MontageTask = Cast<ULogicTask_PlayMontageAndWait>(ActiveTask))
			{
				MontageTask->AdvanceProgress();
				return true;
			}
		}
	}

	return false;
}

void UHoldingStepLogicModuleBase::ConfigureTask(ULogicTaskBase* Task, const FInteractionContext& Context)
{
	if (!Task) return;

	// Montage 기반 태스크인 경우 기본 속성들 주입
	if (ULogicTask_PlayMontageAndWait* MontageTask = Cast<ULogicTask_PlayMontageAndWait>(Task))
	{
		MontageTask->MaxStep = MaxStep;
		MontageTask->ActionTag = ActionTag;

		// 거리 제한 전달 (태스크가 자체적으로 모니터링)
		MontageTask->MaxInteractionDistance = MaxInteractionDistance;

		// [진행도 복구] 아이템의 블랙보드에서 기존 진행도 검색
		if (UInteractablePropertyComponent* InteractableProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp))
		{
			if (AItemBase* StoredItem = InteractableProp->GetStoredItem(SlotTag))
			{
				if (const FItemStatValue* SavedProgress = StoredItem->FindStat(ProgressStatTag))
				{
					MontageTask->CurrentStep = SavedProgress->FloatValue;
					UE_LOG(LogTemp, Log, TEXT("[HoldingStepLogic] 기존 진행도를 복구했습니다: %f"), MontageTask->CurrentStep);
				}
			}
		}
	}
}

ULogicContextComponent* UHoldingStepLogicModuleBase::GetContextComponent(const FInteractionContext& Context) const
{
	if (Context.TargetActor)
	{
		return Context.TargetActor->FindComponentByClass<ULogicContextComponent>();
	}
	return nullptr;
}

void UHoldingStepLogicModuleBase::HandleTaskProgressUpdated(ULogicTaskBase* TaskInstance, float InCurrentStep, float InMaxStep)
{
	if (!TaskInstance) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UInteractionManager* InteractionManager = World->GetSubsystem<UInteractionManager>();
	if (!InteractionManager) return;

	if (TaskInstance)
	{
		const FInteractionContext& Context = TaskInstance->GetCachedContext();
		if (UInteractablePropertyComponent* TargetProp = Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp))
		{
			// 1. UI 갱신 요청 (Atomic Safe API)
			InteractionManager->SafeUpdateProgressUI(TargetProp, ProgressStatTag, MaxProgressStatTag, InCurrentStep, InMaxStep, SlotTag);

			// 2. 데이터 저장 요청 (Atomic Safe API)
			if (ProgressStatTag.IsValid())
			{
				if (AItemBase* TargetItem = TargetProp->GetStoredItem(SlotTag))
				{
					InteractionManager->SafeUpdateItemStat(TargetItem, ProgressStatTag, InCurrentStep);
				}
			}
		}
	}
}

void UHoldingStepLogicModuleBase::HandleTaskCompleted(ULogicTaskBase* TaskInstance)
{
	if (!TaskInstance) return;

	if (TaskInstance)
	{
		const FInteractionContext& Context = TaskInstance->GetCachedContext();

		// 작업 완료: 이동 차단 해제 (타이머 정지는 Task의 EndTask에서 수행)
		if (UInteractorComponent* InteractorComp = Cast<UInteractorComponent>(Context.InteractorComp))
		{
			InteractorComp->SetIsWorking(false);
		}

		// 성공 이벤트 호출
		OnHoldingCompleted(Context);
		
		if (ULogicContextComponent* ContextComponent = GetContextComponent(Context))
		{
			ContextComponent->ClearTask(TaskTag);
		}
	}
}

void UHoldingStepLogicModuleBase::HandleTaskCanceled(ULogicTaskBase* TaskInstance)
{
	if (!TaskInstance) return;

	if (TaskInstance)
	{
		const FInteractionContext& Context = TaskInstance->GetCachedContext();

		// 작업 취소: 이동 차단 해제 (타이머 정지는 Task의 EndTask에서 수행)
		if (UInteractorComponent* InteractorComp = Cast<UInteractorComponent>(Context.InteractorComp))
		{
			InteractorComp->SetIsWorking(false);
		}

		// 취소 이벤트 호출
		OnHoldingCanceled(Context);

		if (ULogicContextComponent* ContextComponent = GetContextComponent(Context))
		{
			ContextComponent->ClearTask(TaskTag);
		}
	}
}

void UHoldingStepLogicModuleBase::OnHoldingStarted_Implementation(const FInteractionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("[HoldingStepLogic] Holding Started - ActionTag: %s"), *ActionTag.ToString());
}

void UHoldingStepLogicModuleBase::OnHoldingCompleted_Implementation(const FInteractionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("[HoldingStepLogicModuleBase] Holding Completed!"));
}

void UHoldingStepLogicModuleBase::OnHoldingCanceled_Implementation(const FInteractionContext& Context)
{
	UE_LOG(LogTemp, Warning, TEXT("[HoldingStepLogic] Holding Canceled!"));
	// 취소 시에는 진행도가 남아있으므로 ProgressWidget을 유지합니다.
	// 완료 시에만 UI를 숨깁니다.
}

