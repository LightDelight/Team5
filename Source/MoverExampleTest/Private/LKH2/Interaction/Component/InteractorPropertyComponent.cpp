// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"

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
