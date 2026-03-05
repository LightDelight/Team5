// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeActorBase.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AMergeActorBase::AMergeActorBase() : Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshCompoent"));
	SetRootComponent(RootStaticMesh);
}

// Called when the game starts or when spawned
void AMergeActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMergeActorBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(IsValid(RootStaticMesh));
	
}

// Called every frame
void AMergeActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

