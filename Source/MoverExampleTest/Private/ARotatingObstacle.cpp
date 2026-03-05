// Fill out your copyright notice in the Description page of Project Settings.


#include "ARotatingObstacle.h"

// Sets default values
AARotatingObstacle::AARotatingObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AARotatingObstacle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AARotatingObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

