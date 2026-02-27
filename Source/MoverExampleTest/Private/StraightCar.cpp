#include "StraightCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AStraightCar::AStraightCar()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;

	BoxCollision->SetCollisionProfileName(TEXT("BlockAll"));
	BoxCollision->SetBoxExtent(FVector(100.f, 50.f, 50.f));

	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
	CarMesh->SetupAttachment(RootComponent);

	MoveSpeed = 500.0f;
	ResetDistance = 3000.0f;
	bIsReturning = false; // 처음엔 출발 상태
}

void AStraightCar::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
}

void AStraightCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 무조건 '내 앞'으로 전진 
	FVector CurrentLocation = GetActorLocation();
	FVector ForwardMove = GetActorForwardVector() * MoveSpeed * DeltaTime;
	SetActorLocation(CurrentLocation + ForwardMove);

	// 2. 시작점과의 거리 계산
	float Distance = FVector::Dist(StartLocation, CurrentLocation);

	// 3. 상태 변경 로직 
	if (!bIsReturning)
	{
		
		if (Distance >= ResetDistance)
		{
			// 180도 회전 (뒤로 돌기)
			AddActorLocalRotation(FRotator(0.0f, 180.0f, 0.0f));
			bIsReturning = true; // 이제 복귀 모드
		}
	}
	else
	{
		// [복귀 중] -> 다시 앞으로 돌아
		if (Distance <= 50.0f) // 오차 범위 50
		{
			// 180도 회전 (다시 앞을 봄)
			AddActorLocalRotation(FRotator(0.0f, 180.0f, 0.0f));
			bIsReturning = false; // 다시 출발 모드
		}
	}
}