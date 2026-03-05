#include "LKH2/Core/StageFinishLineActor.h"
#include "LKH2/Core/StageGameMode.h"
#include "LKH2/Core/CartGameComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AStageFinishLineActor::AStageFinishLineActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	FinishZone = CreateDefaultSubobject<UBoxComponent>(TEXT("FinishZone"));
	RootComponent = FinishZone;
	FinishZone->SetCollisionProfileName(TEXT("Trigger"));
}

void AStageFinishLineActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FinishZone->OnComponentBeginOverlap.AddDynamic(this, &AStageFinishLineActor::OnFinishZoneOverlap);
	}
}

void AStageFinishLineActor::OnFinishZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	// 카트인지 판별 (CartGameComponent를 가지고 있다면 카트로 간주)
	if (OtherActor->FindComponentByClass<UCartGameComponent>())
	{
		if (AStageGameMode* SGM = Cast<AStageGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			SGM->OnCrossedFinishLine();
		}
	}
}
