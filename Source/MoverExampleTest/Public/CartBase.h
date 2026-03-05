// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Physics/NetworkPhysicsComponent.h"
#include "CartBase.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionInstance;

UCLASS()
class MOVEREXAMPLETEST_API ACartBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACartBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PawnClientRestart() override;

public:
	// Called every frame
	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void UnPossessed() override;


private:
	class FPhysicsCart* PhysicsCart;

	UPROPERTY()
	TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Context", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	TObjectPtr<UInputAction> IA_MovementInput;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	TObjectPtr<UInputAction> IA_SteeringInput;

	void OnMovementInput(const FInputActionInstance& Instance);
	void OnSteeringInput(const FInputActionInstance& Instance);

	FVector MovementInput_External{ 0,0,0 };
	float SteeringInput_External = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Input|Input Multiplier")
	float MovementInput_Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Input|Input Multiplier")
	float SteeringInput_Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Input|Input Multiplier")
	float TractionLost_Multiplier = 0.1f;
};

#pragma region NetInput

USTRUCT()
struct FNetInputCart : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetInputCart()
		: MovementInput(0.0f), SteeringInput(0.0f)
	{
	}

	UPROPERTY()
	FVector MovementInput;
	UPROPERTY()
	float SteeringInput;

	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override;
	void MergeData(const FNetworkPhysicsPayload& FromData) override;
	void DecayData(float DecayAmount) override;
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override;
	const FString DebugData() const override { return FString::Printf(TEXT("MovementInput:  - SteeringInput: ")); }
};

USTRUCT()
struct FNetStateCart : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetStateCart() {}
	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override {}
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override { return true; }

};

#pragma endregion NetInput


#pragma region AsyncInput

struct FAsyncInputCart : public Chaos::FSimCallbackInput
{
	FVector MovementInput;
	float SteeringInput;
	void Reset()
	{
		MovementInput = FVector(0.0f);
		SteeringInput = 0.0f;
	}
};

struct FAsyncOutputCart : public Chaos::FSimCallbackOutput
{
	void Reset() {}
};

class FPhysicsCart : public Chaos::TSimCallbackObject<FAsyncInputCart, FAsyncOutputCart,
	(Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::PhysicsObjectUnregister | Chaos::ESimCallbackOptions::Rewind)>
	, TNetworkPhysicsInputState_Internal<FNetInputCart, FNetStateCart>
{
	friend ACartBase;
	~FPhysicsCart() {}

	virtual void OnPostInitialize_Internal() override;
	virtual void ProcessInputs_Internal(int32 PhysicsStep) override;
	virtual void OnPreSimulate_Internal() override;
	virtual void OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject) override;

	virtual void BuildInput_Internal(FNetInputCart& Input) const override;
	virtual void ValidateInput_Internal(FNetInputCart& Input) const override;
	virtual void ApplyInput_Internal(const FNetInputCart& Input) override;
	virtual void BuildState_Internal(FNetStateCart& State) const override;
	virtual void ApplyState_Internal(const FNetStateCart& State) override;


private:
	Chaos::FConstPhysicsObjectHandle PhysicsObject = nullptr;
	FVector MovementInput_Internal;
	float SteeringInput_Internal;

public:
	void SetMovementInput_Internal(FVector InMovementInput) { MovementInput_Internal = InMovementInput; }
	const FVector GetMovementInput_Internal() const { return MovementInput_Internal; }
	void SetSteeringInput_Internal(float InSteeringInput) { SteeringInput_Internal = InSteeringInput; }
	const float GetSteeringInput_Internal() const { return SteeringInput_Internal; }

};

#pragma endregion AsyncInput

