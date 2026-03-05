// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeBoxActor.h"
#include "Components/BoxComponent.h"


AMergeBoxActor::AMergeBoxActor() : Super()
{

	//maybe sphere is cheaper and universal? rootmesh provides physics hit.
	MergeRange = CreateDefaultSubobject<UBoxComponent>(TEXT("Sphere Collision Merge Range"));
	MergeRange->SetupAttachment(RootStaticMesh);
}

void AMergeBoxActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMergeBoxActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(IsValid(MergeRange));
	check(MergeRange->OnComponentBeginOverlap.IsBound());

}

void AMergeBoxActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMergeBoxActor::OnMergeRangeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{

}

