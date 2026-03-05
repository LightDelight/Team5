// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawnBase.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionInstance;
struct FInputActionValue;

UCLASS()
class MOVEREXAMPLETEST_API APlayerPawnBase : public APawn, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawnBase();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	void OnMovementInput(const FInputActionInstance& Instance);

protected:
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override final;

	virtual void OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult);

protected:
	UFUNCTION(BlueprintCallable)
	void AfflictImpulse_Client(AActor* TargetActor);

	UFUNCTION(Server, Reliable)
	void AfflictImpulse_Server(int32 CorrespondingFrame, AActor* TargetActor, FVector ImpulseDirection);

	void ImpulseAfflictor(int32 CorrespondingFrame, AActor* TargetActor, FVector ImpulseDirection);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Context", meta = (AllowPrivateAccess = true));
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	TObjectPtr<UInputAction> IA_MovementInput;

	FVector MovementInput_External{ 0,0,0 };


};
