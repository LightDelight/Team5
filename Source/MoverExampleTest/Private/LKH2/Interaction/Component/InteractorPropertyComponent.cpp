// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UInteractorPropertyComponent::UInteractorPropertyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UInteractorPropertyComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void UInteractorPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInteractorPropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInteractorPropertyComponent, CarriedActor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInteractorPropertyComponent, CurrentActionTag, COND_None, REPNOTIFY_Always);
}

void UInteractorPropertyComponent::OnRep_CarriedActor(AActor* OldCarriedActor)
{
	if (CarriedActor)
	{
		// 네트워크 지연으로 ItemState(물리 끄기)보다 CarriedActor(부착)가 먼저 OnRep 될 경우 
		// 물리가 켜져 있어서 부착이 무시되는 현상을 방지하기 위해 강제로 물리를 일단 끕니다.
		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(CarriedActor->GetRootComponent()))
		{
			RootPrim->SetSimulatePhysics(false);
		}

		// 1. 물리적 부착 (스냅)
		if (AActor* TargetActor = GetOwner())
		{
			// InteractorProperty의 Owner에서 InteractorComponent를 찾아서 거기로 스냅
			if (UActorComponent* InteractorComp = TargetActor->FindComponentByClass<UInteractorComponent>())
			{
				USceneComponent* InteractorSceneComp = Cast<USceneComponent>(InteractorComp);
				if (InteractorSceneComp)
				{
					CarriedActor->AttachToComponent(
						InteractorSceneComp,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
				else
				{
					CarriedActor->AttachToActor(
						TargetActor,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
			}
			else
			{
				CarriedActor->AttachToActor(
					TargetActor,
					FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}
	else if (OldCarriedActor)
	{
		// 1. 물리적 해제
		OldCarriedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

AActor* UInteractorPropertyComponent::GetCarriedActor() const
{
	return CarriedActor;
}

void UInteractorPropertyComponent::ForceEquip(AActor* ItemToEquip)
{
	if (ItemToEquip && CarriedActor != ItemToEquip)
	{
		AActor* OldItem = CarriedActor;
		CarriedActor = ItemToEquip;

		// 물리 부착은 OnRep_CarriedActor에서 일괄 처리되도록 위임

		// 클라이언트 예측 혹은 서버 실행 모두 로컬 부착/탈착을 동기화해야 하므로 무조건 호출
		OnRep_CarriedActor(OldItem);
	}
}

void UInteractorPropertyComponent::ForceDrop()
{
	if (CarriedActor)
	{
		AActor* OldItem = CarriedActor;
		CarriedActor = nullptr;

		// 1. 물리적 해제는 OnRep_CarriedActor에서 수행되지만,
		// 위치 보정을 위해 서버/로컬 주체에서 먼저 Detach 할 수 있습니다. 
		// (안전을 위해 Detach 위치를 유지한 채 여기서 수행)
		OldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		if (AActor* InteractorOwner = GetOwner())
		{
			FVector SafeLoc = InteractorOwner->GetActorLocation() +
							  InteractorOwner->GetActorForwardVector() * 80.0f;
			SafeLoc.Z = OldItem->GetActorLocation().Z;
			OldItem->SetActorLocation(SafeLoc);
		}

		// 클라이언트 예측 혹은 서버 실행 모두 로컬 부착/탈착을 동기화해야 하므로 무조건 호출
		OnRep_CarriedActor(OldItem);
	}
}

/* ---------------------------------------------------------
 * Action & Montage 
 * --------------------------------------------------------- */

void UInteractorPropertyComponent::OnRep_CurrentActionTag(FGameplayTag OldTag)
{
	UE_LOG(LogTemp, Log, TEXT("[InteractorProperty] OnRep_CurrentActionTag 호출됨. CompName: %s, OldTag: %s, NewTag: %s, Owner: %s"), *GetName(), *OldTag.ToString(), *CurrentActionTag.ToString(), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));

	if (CurrentActionTag.IsValid())
	{
		PlayMontageForTag(CurrentActionTag);
	}
	else
	{
		StopCurrentMontage();
	}
}

void UInteractorPropertyComponent::SetActionTag(FGameplayTag NewTag)
{
	UE_LOG(LogTemp, Log, TEXT("[InteractorProperty] SetActionTag 호출됨. (CompName: %s, 요청: %s, 현재: %s, Owner: %s)"), *GetName(), *NewTag.ToString(), *CurrentActionTag.ToString(), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));

	if (CurrentActionTag != NewTag)
	{
		FGameplayTag OldTag = CurrentActionTag;
		CurrentActionTag = NewTag;
		
		// 서버에서 직접 호출했을 때도 로컬(호스트) 동작을 위해 OnRep 호출
		OnRep_CurrentActionTag(OldTag);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[InteractorProperty] 이미 액션 태그가 일치하므로 무시합니다."));
	}
}

void UInteractorPropertyComponent::ClearActionTag()
{
	SetActionTag(FGameplayTag::EmptyTag);
}

void UInteractorPropertyComponent::PlayMontageForTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return;

	UAnimMontage** MontagePtr = ActionMontageMap.Find(Tag);
	if (!MontagePtr || !(*MontagePtr))
	{
		UE_LOG(LogTemp, Error, TEXT("[InteractorProperty] PlayMontageForTag 실패: Map에 %s에 대한 몽타주가 존재하지 않습니다. (CompName: %s)"), *Tag.ToString(), *GetName());
		// 등록된 몽타주가 없으면 무시
		return;
	}

	UAnimMontage* MontageToPlay = *MontagePtr;

	if (AActor* OwnerActor = GetOwner())
	{
		if (USkeletalMeshComponent* MeshComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				StopCurrentMontage(); // 혹시 기존 재생 중이던 것이 있다면 정지
				float Duration = AnimInst->Montage_Play(MontageToPlay);
				UE_LOG(LogTemp, Log, TEXT("[InteractorProperty] 몽타주 재생 호출 완료! (몽타주: %s, 길이: %f)"), *MontageToPlay->GetName(), Duration);
				ActiveMontage = MontageToPlay;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[InteractorProperty] PlayMontageForTag 실패: AnimInstance를 찾을 수 없습니다."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[InteractorProperty] PlayMontageForTag 실패: USkeletalMeshComponent를 찾을 수 없습니다."));
		}
	}
}

void UInteractorPropertyComponent::StopCurrentMontage()
{
	if (ActiveMontage)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			if (USkeletalMeshComponent* MeshComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
				{
					AnimInst->Montage_Stop(0.2f, ActiveMontage);
				}
			}
		}
		ActiveMontage = nullptr;
	}
}
