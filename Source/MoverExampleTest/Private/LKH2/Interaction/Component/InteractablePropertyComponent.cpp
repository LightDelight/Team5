// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "GameFramework/GameStateBase.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/UI/Widget/LogicProgressWidget.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"

// Sets default values for this component's properties
UInteractablePropertyComponent::UInteractablePropertyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // DOREPLIFETIME / OnRep 동작을 위해 필수
	
	ProgressWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LogicProgressWidget"));
	if (ProgressWidgetComponent)
	{
		ProgressWidgetComponent->SetupAttachment(this);
		ProgressWidgetComponent->SetWidgetSpace(EWidgetSpace::World); // 3D 빌보드: Workstation 위에 월드 공간으로 표시
		ProgressWidgetComponent->SetDrawSize(FVector2D(100.0f, 15.0f));
		ProgressWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // 120cm 위로
	}
}

void UInteractablePropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractablePropertyComponent, UIState);
}

// Called when the game starts
void UInteractablePropertyComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ProgressWidgetComponent && GetOwner())
	{
		// PropertyComponent(this)가 아닌 Owner의 RootComponent에 직접 Attach하여
		// SpawnActorDeferred 흐름에서도 World Transform 추적을 보장합니다.
		ProgressWidgetComponent->AttachToComponent(
			GetOwner()->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
		ProgressWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
		ProgressWidgetComponent->SetVisibility(false);
	}
}

// Called every frame
void UInteractablePropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInteractablePropertyComponent::TryStoreItem(FGameplayTag SlotTag, AItemBase* ItemToStore)
{
	if (!SlotTag.IsValid() || !ItemToStore)
	{
		return false;
	}

	// 이미 해당 슬롯에 누군가 거치되어 있다면 실패
	if (StoredItems.Contains(SlotTag) && StoredItems[SlotTag] != nullptr)
	{
		return false;
	}

	StoredItems.Add(SlotTag, ItemToStore);

	// 시스템 의도 발행 (자율 로직 트리거)
	if (ItemAddedIntentTag.IsValid())
	{
		if (IInteractionContextInterface* ContextOwner = Cast<IInteractionContextInterface>(GetOwner()))
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Issuing ItemAdded Intent: %s for Slot: %s on Actor: %s"), 
				*GetName(), *ItemAddedIntentTag.ToString(), *SlotTag.ToString(), *GetOwner()->GetName());

			FInteractionContext SystemContext;
			SystemContext.TargetActor = GetOwner();
			SystemContext.InteractionTag = ItemAddedIntentTag;
			SystemContext.ItemUID = ItemToStore->GetInstanceId();
			SystemContext.Interactor = ItemToStore;
			// 슬롯 정보 추가 (로직에서 어떤 슬롯에 아이템이 들어왔는지 알 수 있게 함)
			SystemContext.SlotTag = SlotTag;
			SystemContext.ContextComp = GetOwner()->FindComponentByClass<ULogicContextComponent>();
			
			IInteractionContextInterface::Execute_OnInteract(GetOwner(), SystemContext);
		}
	}

	return true;
}

AItemBase* UInteractablePropertyComponent::RetrieveItem(FGameplayTag SlotTag)
{
	if (!SlotTag.IsValid())
	{
		return nullptr;
	}

	if (StoredItems.Contains(SlotTag) && StoredItems[SlotTag] != nullptr)
	{
		AItemBase* Item = StoredItems[SlotTag];
		StoredItems[SlotTag] = nullptr;

		// 만약 제거되는 아이템이 현재 UI와 연동된 아이템이라면 UI 제거
		if (UIState.Mode != EProgressDisplayMode::None && UIState.LinkedItemUID.IsValid() && UIState.LinkedItemUID == Item->GetInstanceId())
		{
			InternalClearUI();
		}

		// 시스템 의도 발행 (자율 로직 트리거) - 취소/중단용
		if (ItemRemovedIntentTag.IsValid())
		{
			if (IInteractionContextInterface* ContextOwner = Cast<IInteractionContextInterface>(GetOwner()))
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] Issuing ItemRemoved Intent: %s for Slot: %s on Actor: %s"), 
					*GetName(), *ItemRemovedIntentTag.ToString(), *SlotTag.ToString(), *GetOwner()->GetName());

				FInteractionContext SystemContext;
				SystemContext.TargetActor = GetOwner();
				SystemContext.InteractionTag = ItemRemovedIntentTag;
				SystemContext.Interactor = Item;
				SystemContext.ItemUID = Item->GetInstanceId();

				IInteractionContextInterface::Execute_OnInteract(GetOwner(), SystemContext);
			}
		}

		return Item;
	}

	return nullptr;
}

AItemBase* UInteractablePropertyComponent::GetStoredItem(FGameplayTag SlotTag) const
{
	if (StoredItems.Contains(SlotTag))
	{
		return StoredItems[SlotTag];
	}
	return nullptr;
}

bool UInteractablePropertyComponent::HasItem(FGameplayTag SlotTag) const
{
	if (StoredItems.Contains(SlotTag))
	{
		return StoredItems[SlotTag] != nullptr;
	}
	return false;
}

void UInteractablePropertyComponent::AttachTargetItem(AItemBase* ItemToStore, USceneComponent* SnapComponent)
{
	if (!ItemToStore)
	{
		return;
	}

	if (SnapComponent)
	{
		// 지정된 SceneComponent 위치(예: InteractableComponent)에 스냅
		ItemToStore->AttachToComponent(
			SnapComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else if (AActor* TargetActor = GetOwner())
	{
		// fallback: 소유 액터의 루트에 스냅
		ItemToStore->AttachToActor(
			TargetActor,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void UInteractablePropertyComponent::DetachTargetItem(AItemBase* ItemToStore)
{
	if (!ItemToStore)
	{
		return;
	}

	ItemToStore->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void UInteractablePropertyComponent::ShowStepProgressUI(FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag)
{
	InternalSetStepUI(CurrentStepTag, MaxStepTag, 0.0f, 100.0f); // 기본값
}

void UInteractablePropertyComponent::InternalSetTimerUI(FGameplayTag InStartTag, FGameplayTag InEndTag, FGuid InItemUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UIState.Mode = EProgressDisplayMode::Timer;
		UIState.StartTimeTag = InStartTag;
		UIState.EndTimeTag = InEndTag;
		UIState.CurrentStepTag = FGameplayTag::EmptyTag;
		UIState.MaxStepTag = FGameplayTag::EmptyTag;
		UIState.LinkedItemUID = InItemUID;
		
		OnRep_UIState();
	}
}

void UInteractablePropertyComponent::InternalSetStepUI(FGameplayTag InCurrentTag, FGameplayTag InMaxTag, float InCurrent, float InMax, FGuid InItemUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UIState.Mode = EProgressDisplayMode::Step;
		UIState.CurrentStepTag = InCurrentTag;
		UIState.MaxStepTag = InMaxTag;
		UIState.CurrentStep = InCurrent;
		UIState.MaxStep = InMax;
		UIState.StartTimeTag = FGameplayTag::EmptyTag;
		UIState.EndTimeTag = FGameplayTag::EmptyTag;
		UIState.LinkedItemUID = InItemUID;

		OnRep_UIState();
	}
}

void UInteractablePropertyComponent::InternalFreezeTimerToStep(FGameplayTag InCurrentTag, FGameplayTag InMaxTag)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (UIState.Mode != EProgressDisplayMode::Timer) return;

	// 현재 시점의 비율 계산
	float CurrentRatio = 0.0f;
	float Duration = 1.0f;

	AActor* StatOwner = GetOwner();
	if (UIState.LinkedItemUID.IsValid())
	{
		if (UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>())
		{
			if (AActor* ItemActor = ItemManager->GetItemActor(UIState.LinkedItemUID))
			{
				StatOwner = ItemActor;
			}
		}
	}

	if (StatOwner)
	{
		if (ILogicContextInterface* Context = Cast<ILogicContextInterface>(StatOwner))
		{
			const FItemStatValue* StartStat = Context->FindStat(UIState.StartTimeTag);
			const FItemStatValue* EndStat = Context->FindStat(UIState.EndTimeTag);

			if (StartStat && EndStat && StartStat->FloatValue > 0 && EndStat->FloatValue > StartStat->FloatValue)
			{
				float StartTime = StartStat->FloatValue;
				float EndTime = EndStat->FloatValue;
				Duration = EndTime - StartTime;
				
				float CurrentTime = 0.0f;
				if (AGameStateBase* GS = GetWorld()->GetGameState())
				{
					CurrentTime = GS->GetServerWorldTimeSeconds();
				}
				else
				{
					CurrentTime = GetWorld()->GetTimeSeconds();
				}

				CurrentRatio = FMath::Clamp(CurrentTime - StartTime, 0.0f, Duration);

				// 타이머 블랙보드 스탯 초기화
				FItemStatValue ClearStat;
				ClearStat.Type = EItemStatType::Float;
				ClearStat.FloatValue = -1.0f;
				Context->SetStat(UIState.StartTimeTag, ClearStat);
				Context->SetStat(UIState.EndTimeTag, ClearStat);
				
				// 스텝 블랙보드 스탯 저장
				FItemStatValue Val; Val.Type = EItemStatType::Float; Val.FloatValue = CurrentRatio;
				Context->SetStat(InCurrentTag, Val);
				Val.FloatValue = Duration;
				Context->SetStat(InMaxTag, Val);
			}
		}
	}

	// Step 모드로 전환 (계산된 최종 수치 고정)
	InternalSetStepUI(InCurrentTag, InMaxTag, CurrentRatio, Duration, UIState.LinkedItemUID);
}

void UInteractablePropertyComponent::InternalClearUI()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UIState.Mode = EProgressDisplayMode::None;
		UIState.StartTimeTag = FGameplayTag::EmptyTag;
		UIState.EndTimeTag = FGameplayTag::EmptyTag;
		UIState.CurrentStepTag = FGameplayTag::EmptyTag;
		UIState.MaxStepTag = FGameplayTag::EmptyTag;
		UIState.LinkedItemUID = FGuid();

		OnRep_UIState();
	}
}

void UInteractablePropertyComponent::OnRep_UIState()
{
	if (!ProgressWidgetComponent) return;

	if (UIState.Mode == EProgressDisplayMode::None)
	{
		ProgressWidgetComponent->SetVisibility(false);
		return;
	}

	if (UIState.Mode != EProgressDisplayMode::None)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] OnRep_UIState - Mode: %d, LinkedItemUID: %s"), 
			*GetName(), (int32)UIState.Mode, *UIState.LinkedItemUID.ToString());
	}

	if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			// 위젯에 태그 전달 및 초기화
			LogicWidget->StartTimeTag = UIState.StartTimeTag;
			LogicWidget->EndTimeTag = UIState.EndTimeTag;
			LogicWidget->CurrentStepTag = UIState.CurrentStepTag;
			LogicWidget->MaxStepTag = UIState.MaxStepTag;
			
			LogicWidget->InitializeProgressWidget(this);
		}
	}

	ProgressWidgetComponent->SetVisibility(true);
}

void UInteractablePropertyComponent::ShowProgressUI(FGameplayTag StartTimeTag, FGameplayTag EndTimeTag)
{
	InternalSetTimerUI(StartTimeTag, EndTimeTag, FGuid());
}

void UInteractablePropertyComponent::HideProgressUI()
{
	InternalClearUI();
}


void UInteractablePropertyComponent::LockProgressUI()
{
	if (!ProgressWidgetComponent)
	{
		return;
	}

	if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			LogicWidget->LockProgress();
		}
	}
}

void UInteractablePropertyComponent::UnlockProgressUI()
{
	if (!ProgressWidgetComponent)
	{
		return;
	}

	if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			LogicWidget->UnlockProgress();
		}
	}
}

void UInteractablePropertyComponent::ResetProgressUI()
{
	if (!ProgressWidgetComponent)
	{
		return;
	}

	if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			LogicWidget->ResetProgress();
		}
	}
}

