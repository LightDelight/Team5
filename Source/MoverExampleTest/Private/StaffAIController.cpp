#include "StaffAIController.h"
#include "Kismet/GameplayStatics.h"

AStaffAIController::AStaffAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStaffAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChaseUpdateTimer += DeltaTime;
	if (ChaseUpdateTimer >= ChaseUpdateInterval)
	{
		
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			
			MoveToActor(PlayerPawn, 30.0f);
		}
		ChaseUpdateTimer = 0.0f;
	}
}