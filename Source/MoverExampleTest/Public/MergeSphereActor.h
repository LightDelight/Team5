// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MergeActorBase.h"
#include "MergeSphereActor.generated.h"


class USphereComponent;
/**
 *
 */
UCLASS()
class MOVEREXAMPLETEST_API AMergeSphereActor : public AMergeActorBase
{
	GENERATED_BODY()

public:
	AMergeSphereActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:
	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnMergeRangeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> MergeRange{ nullptr };



};

