#include "StaffCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AStaffCharacter::AStaffCharacter()
{
	
	bUseControllerRotationYaw = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AStaffCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->OnComponentHit.AddDynamic(this, &AStaffCharacter::OnEmployeeHit);
	}
}

void AStaffCharacter::OnEmployeeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	if (OtherActor && OtherActor != this && OtherComp && OtherComp->IsSimulatingPhysics())
	{
		
		FVector PushDirection = OtherActor->GetActorLocation() - GetActorLocation();
		PushDirection.Z = 0.2f; 
		PushDirection.Normalize();

		
		OtherComp->AddImpulse(PushDirection * PushStrength);
	}
}