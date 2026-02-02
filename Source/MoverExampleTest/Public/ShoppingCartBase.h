// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Physics/NetworkPhysicsComponent.h"
#include "ShoppingCartBase.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionInstance;

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsPawn, All, All);

UCLASS()
class MOVEREXAMPLETEST_API AShoppingCartBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShoppingCartBase();

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
	/*UFUNCTION(BlueprintCallable, Category = "Game|PhysicsPawn")
	*/void SetForwardInput(const float InForwardInput);



private:
	class FPhysicsShoppingCart* PhysicsShoppingCart;

	UPROPERTY()
	TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_MovementInput;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_CrabInput;

	void On_MovementInput(const FInputActionInstance& Instance);
	void On_CrabInputInput(const FInputActionInstance& Instance);

	float ForwardInput_External;
	float RightInput_External;
	float CrabInput_External;
};

#pragma region NetInput

USTRUCT()
struct FNetInputShoppingCart : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetInputShoppingCart()
		: MovementInput(0.0f), SteeringInput(0.0f)
	{
	}

	UPROPERTY()
	float MovementInput;
	UPROPERTY()
	float SteeringInput;
	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override;
	void MergeData(const FNetworkPhysicsPayload& FromData) override;
	void DecayData(float DecayAmount) override;
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override;
	const FString DebugData() const override { return FString::Printf(TEXT("MovementInput: %f - SteeringInput: %f"), MovementInput, SteeringInput); }
};

USTRUCT()
struct FNetStateShoppingCart : public FNetworkPhysicsPayload
{
	GENERATED_BODY()

	FNetStateShoppingCart() {}
	void InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha) override {}
	bool CompareData(const FNetworkPhysicsPayload& PredictedData) const override { return true; }

};

#pragma endregion NetInput


#pragma region AsyncInput

struct FAsyncInputShoppingCart : public Chaos::FSimCallbackInput
{
	float MovementInput;
	float SteeringInput;
	void Reset()
	{
		MovementInput = 0.0f;
		SteeringInput = 0.0f;
	}
};

struct FAsyncOutputShoppingCart : public Chaos::FSimCallbackOutput
{
	void Reset() {}
};

class FPhysicsShoppingCart : public Chaos::TSimCallbackObject<FAsyncInputShoppingCart, FAsyncOutputShoppingCart,
	(Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::PhysicsObjectUnregister | Chaos::ESimCallbackOptions::Rewind)>
	, TNetworkPhysicsInputState_Internal<FNetInputShoppingCart, FNetStateShoppingCart>
{
	friend AShoppingCartBase;
	~FPhysicsShoppingCart() {}

	virtual void OnPostInitialize_Internal() override;
	virtual void ProcessInputs_Internal(int32 PhysicsStep) override;
	virtual void OnPreSimulate_Internal() override;
	virtual void OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject) override;

	virtual void BuildInput_Internal(FNetInputShoppingCart& Input) const override;
	virtual void ValidateInput_Internal(FNetInputShoppingCart& Input) const override;
	virtual void ApplyInput_Internal(const FNetInputShoppingCart& Input) override;
	virtual void BuildState_Internal(FNetStateShoppingCart& State) const override;
	virtual void ApplyState_Internal(const FNetStateShoppingCart& State) override;


private:
	Chaos::FConstPhysicsObjectHandle PhysicsObject = nullptr;
	float MovementInput_Internal;
	float SteeringInput_Internal;

public:
	void SetMovementInput_Internal(float InMovementInput) { MovementInput_Internal = InMovementInput; }
	const float GetMovementInput_Internal() const { return MovementInput_Internal; }
	void SetSteeringInput_Internal(float InSteeringInput) { SteeringInput_Internal = InSteeringInput; }
	const float GetSteeringInput_Internal() const { return SteeringInput_Internal; }

};

#pragma endregion AsyncInput
