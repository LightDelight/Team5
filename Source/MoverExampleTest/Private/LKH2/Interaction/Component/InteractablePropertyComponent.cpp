// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/UI/Widget/LogicProgressWidget.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

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
	DOREPLIFETIME(UInteractablePropertyComponent, bProgressVisible);
	DOREPLIFETIME(UInteractablePropertyComponent, RepCurrentStepTag);
	DOREPLIFETIME(UInteractablePropertyComponent, RepMaxStepTag);
	DOREPLIFETIME(UInteractablePropertyComponent, RepCurrentStep);
	DOREPLIFETIME(UInteractablePropertyComponent, RepMaxStep);
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
			UE_LOG(LogTemp, Log, TEXT("[%s] Issuing ItemAdded Intent: %s for Slot: %s"), *GetName(), *ItemAddedIntentTag.ToString(), *SlotTag.ToString());

			FInteractionContext SystemContext;
			SystemContext.TargetActor = GetOwner();
			SystemContext.InteractionTag = ItemAddedIntentTag;
			// Interactor가 없는 시스테믹 컨텍스트임을 명시
			
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

		// 시스템 의도 발행 (자율 로직 트리거 - 취소/중단용)
		if (ItemRemovedIntentTag.IsValid())
		{
			if (IInteractionContextInterface* ContextOwner = Cast<IInteractionContextInterface>(GetOwner()))
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] Issuing ItemRemoved Intent: %s for Slot: %s"), *GetName(), *ItemRemovedIntentTag.ToString(), *SlotTag.ToString());

				FInteractionContext SystemContext;
				SystemContext.TargetActor = GetOwner();
				SystemContext.InteractionTag = ItemRemovedIntentTag;
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
	UE_LOG(LogTemp, Log, TEXT("[ShowStepProgressUI] 호출됨 - Owner=%s, CurrentStepTag=%s, MaxStepTag=%s"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("nullptr"),
		*CurrentStepTag.ToString(), *MaxStepTag.ToString());

	if (!ProgressWidgetComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowStepProgressUI] ProgressWidgetComponent가 nullptr"));
		return;
	}

	UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget();
	UE_LOG(LogTemp, Log, TEXT("[ShowStepProgressUI] GetWidget() = %s"),
		UserWidget ? *UserWidget->GetClass()->GetName() : TEXT("nullptr"));

	if (UserWidget)
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			// Step 기반: CurrentStepTag와 MaxStepTag를 Widget에 올바르게 설정
			LogicWidget->CurrentStepTag = CurrentStepTag;
			LogicWidget->MaxStepTag = MaxStepTag;
			LogicWidget->InitializeProgressWidget(this);
			UE_LOG(LogTemp, Log, TEXT("[ShowStepProgressUI] Widget 태그 설정 완료"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShowStepProgressUI] ULogicProgressWidget로 캐스팅 실패 (실제 클래스: %s)"),
				*UserWidget->GetClass()->GetName());
		}
	}

	// 서버에서 replicated 필드 업데이트 → 클라이언트의 OnRep_ProgressVisible 트리거
	if (GetOwnerRole() == ROLE_Authority)
	{
		RepCurrentStepTag = CurrentStepTag;
		RepMaxStepTag = MaxStepTag;
		bProgressVisible = true;
	}

	ProgressWidgetComponent->SetVisibility(true);
}

void UInteractablePropertyComponent::ShowProgressUI(FGameplayTag StartTimeTag, FGameplayTag EndTimeTag)
{
	if (!ProgressWidgetComponent)
	{
		return;
	}

	if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
	{
		if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
		{
			LogicWidget->StartTimeTag = StartTimeTag;
			LogicWidget->EndTimeTag = EndTimeTag;
			LogicWidget->InitializeProgressWidget(this);
		}
	}

	ProgressWidgetComponent->SetVisibility(true);
}

void UInteractablePropertyComponent::HideProgressUI()
{
	if (ProgressWidgetComponent)
	{
		ProgressWidgetComponent->SetVisibility(false);
	}

	// 서버에서 replicated 필드 false → 클라이언트도 숨김
	if (GetOwnerRole() == ROLE_Authority)
	{
		bProgressVisible = false;
	}
}

// 클라이언트에서 bProgressVisible 변경 시 자동 호출
void UInteractablePropertyComponent::OnRep_ProgressVisible()
{
	if (bProgressVisible)
	{
		// 서버에서 복제된 태그를 사용하여 클라이언트에서 위젯 활성화
		if (ProgressWidgetComponent)
		{
			if (UUserWidget* UserWidget = ProgressWidgetComponent->GetWidget())
			{
				if (ULogicProgressWidget* LogicWidget = Cast<ULogicProgressWidget>(UserWidget))
				{
					LogicWidget->CurrentStepTag = RepCurrentStepTag;
					LogicWidget->MaxStepTag = RepMaxStepTag;
					LogicWidget->InitializeProgressWidget(this);
				}
			}
			ProgressWidgetComponent->SetVisibility(true);
		}
	}
	else
	{
		if (ProgressWidgetComponent)
		{
			ProgressWidgetComponent->SetVisibility(false);
		}
	}
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

