// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsDoor.generated.h"

UCLASS()
class MOVEREXAMPLETEST_API APhysicsDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APhysicsDoor();

protected:
	
	virtual void BeginPlay() override;

public:	
	//1.¹®Æ²
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Door System")
	UStaticMeshComponent* DoorFrame;

	//2. ¹®Â¦
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category ="Door System")
	UStaticMeshComponent* DoorMesh;

	//3.°æÃ¸
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Door System")
	UPhysicsConstraintComponent* DoorHinge;

	UPROPERTY(EditAnywhere,Category="Door System")
	float MaxOpenAngle = 90.0f;

};
