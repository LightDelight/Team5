#include "CatCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

ACatCharacter::ACatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACatCharacter::BeginPlay() { Super::BeginPlay(); }
void ACatCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void ACatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) { Super::SetupPlayerInputComponent(PlayerInputComponent); }


void PlayCatMontage(ACharacter* Character, UAnimMontage* Montage)
{
	if (Character && Montage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(Montage, 1.0f);
		}
	}
}


void ACatCharacter::ThrowInput() { PlayCatMontage(this, ThrowMontage); }
void ACatCharacter::SweepInput() { PlayCatMontage(this, SweepMontage); }
void ACatCharacter::ChopInput() { PlayCatMontage(this, ChopMontage); }
void ACatCharacter::PickupInput() { PlayCatMontage(this, PickupMontage); }


void ACatCharacter::OnThrowImpact_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Throw!")); }
void ACatCharacter::OnSweepImpact_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Sweep!")); }
void ACatCharacter::OnChopImpact_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Chop!")); }
void ACatCharacter::OnPickupImpact_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Pickup!")); }