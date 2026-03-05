// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCommonMacros.h"
#include "GameFramework/Actor.h"
#include "MergeActorBase.generated.h"

class UStaticMeshComponent;

UCLASS(Abstract)
class MOVEREXAMPLETEST_API AMergeActorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMergeActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
	virtual void OnMergeRangeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) PURE_VIRTUAL(&ThisClass::OnMergeRangeOverlap);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> RootStaticMesh{ nullptr };
};
