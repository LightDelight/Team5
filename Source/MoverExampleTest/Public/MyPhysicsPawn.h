// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Physics/NetworkPhysicsComponent.h"
#include "MyPhysicsPawn.generated.h"

UCLASS()
class MOVEREXAMPLETEST_API AMyPhysicsPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyPhysicsPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Game|PhysicsPawn")
	void SetForwardInput(const float InForwardInput);

	UFUNCTION(BlueprintCallable, Category = "Game|PhysicsPawn")
	void SetBackwardInput(const float InBackwardInput);

	UFUNCTION(BlueprintCallable, Category = "Game|PhysicsPawn")
	void SetSteeringInput(const float InSteeringInput);

	UFUNCTION(BlueprintCallable, Category = "Game|PhysicsPawn")
	void SetYawInput(const float InYawInput);

private:
	class FPhysicsPawnAsync* PhysicsPawnAsync;
	
	UPROPERTY()
	TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

	float ForwardInput_External = 0.0f;
	float BackwardInput_External = 0.0f;
	float SteeringInput_External = 0.0f;
	float YawInput_External = 0.0f;

};

#pragma region NetInput

USTRUCT()
struct FNetInputPhysicsPawn : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetInputPhysicsPawn()
		: MovementInput(0.0f), SteeringInput(0.0f)
	{
	}

	UPROPERTY()
	float MovementInput;
	UPROPERTY()
	float SteeringInput;
	UPROPERTY()
	float YawInput;

	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override;
	void MergeData(const FNetworkPhysicsPayload& FromData) override;
	void DecayData(float DecayAmount) override;
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override;
	const FString DebugData() const override { return FString::Printf(TEXT("MovementInput: %f - SteeringInput: %f"), MovementInput, SteeringInput); }
};

USTRUCT()
struct FNetStatePhysicsPawn : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetStatePhysicsPawn() {}
	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override {}
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override { return true; }

};

#pragma endregion NetInput


#pragma region AsyncInput

struct FAsyncInputPhysicsPawn : public Chaos::FSimCallbackInput
{
	float MovementInput;
	float SteeringInput;
	float YawInput;
	void Reset()
	{
		YawInput = 0.0f;
		MovementInput = 0.0f;
		SteeringInput = 0.0f;
	}
};

struct FAsyncOutputPhysicsPawn : public Chaos::FSimCallbackOutput
{
	void Reset() {}
};

class FPhysicsPawnAsync : public Chaos::TSimCallbackObject<FAsyncInputPhysicsPawn, FAsyncOutputPhysicsPawn,
	(Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::PhysicsObjectUnregister | Chaos::ESimCallbackOptions::Rewind)>
	, TNetworkPhysicsInputState_Internal<FNetInputPhysicsPawn,FNetStatePhysicsPawn>
{
	friend AMyPhysicsPawn;
	~FPhysicsPawnAsync() {}

	virtual void OnPostInitialize_Internal() override;
	virtual void ProcessInputs_Internal(int32 PhysicsStep) override;
	virtual void OnPreSimulate_Internal() override;
	virtual void OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject) override;

	virtual void BuildInput_Internal(FNetInputPhysicsPawn& Input) const override;
	virtual void ValidateInput_Internal(FNetInputPhysicsPawn& Input) const override;
	virtual void ApplyInput_Internal(const FNetInputPhysicsPawn& Input) override;
	virtual void BuildState_Internal(FNetStatePhysicsPawn& State) const override;
	virtual void ApplyState_Internal(const FNetStatePhysicsPawn& State) override;


private:
	Chaos::FConstPhysicsObjectHandle PhysicsObject = nullptr;
	float MovementInput_Internal;
	float SteeringInput_Internal;
	float YawInput_Internal;

public:
	void SetMovementInput_Internal(float InMovementInput) { MovementInput_Internal = InMovementInput; }
	const float GetMovementInput_Internal() const { return MovementInput_Internal; }
	void SetSteeringInput_Internal(float InSteeringInput) { SteeringInput_Internal = InSteeringInput; }
	const float GetSteeringInput_Internal() const { return SteeringInput_Internal; }
	void SetYawInput_Internal(float InSteeringInput) { YawInput_Internal = InSteeringInput; }
	const float GetYawInput_Internal() const { return YawInput_Internal; }

};

#pragma endregion AsyncInput

