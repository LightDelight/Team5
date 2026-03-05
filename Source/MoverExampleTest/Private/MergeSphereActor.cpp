// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeSphereActor.h"
#include "Components/SphereComponent.h"

AMergeSphereActor::AMergeSphereActor() : Super()
{
	MergeRange = CreateDefaultSubobject<USphereComponent>(TEXT("Box Collision Merge Range"));
	MergeRange->SetupAttachment(RootStaticMesh);
	MergeRange->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnMergeRangeBeginOverlap);

}

void AMergeSphereActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMergeSphereActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(IsValid(MergeRange));
	check(MergeRange->OnComponentBeginOverlap.IsBound());
}

void AMergeSphereActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMergeSphereActor::OnMergeRangeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	PRINT_FUNCTION_NAME;
	OtherActor->Destroy();
}
