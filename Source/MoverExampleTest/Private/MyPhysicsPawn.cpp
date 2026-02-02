// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPhysicsPawn.h"
#include "Engine/Engine.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "Chaos/PhysicsObjectInternalInterface.h"

// Sets default values
AMyPhysicsPawn::AMyPhysicsPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (UPhysicsSettings::Get()->PhysicsPrediction.bEnablePhysicsPrediction) {
		static const FName NetworkPhysicsComponentName(TEXT("NetworkPhysicsComponent"));
		NetworkPhysicsComponent = CreateDefaultSubobject<UNetworkPhysicsComponent>(NetworkPhysicsComponentName);
		NetworkPhysicsComponent->SetNetAddressable();
		NetworkPhysicsComponent->SetIsReplicated(true);
	}

}

// Called when the game starts or when spawned
void AMyPhysicsPawn::BeginPlay()
{
	Super::BeginPlay();

}

void AMyPhysicsPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMyPhysicsPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UPrimitiveComponent* RootSimulatedComponent = Cast<UPrimitiveComponent>(GetRootComponent())) {
		if (UWorld* World = GetWorld()) {
			if (FPhysScene* PhysScene = World->GetPhysicsScene()) {
				if (Chaos::FPhysicsSolver* Solver = PhysScene->GetSolver()) {
					PhysicsPawnAsync = Solver->CreateAndRegisterSimCallbackObject_External<FPhysicsPawnAsync>();
					if (ensure(PhysicsPawnAsync)) {
						PhysicsPawnAsync->PhysicsObject = RootSimulatedComponent->GetPhysicsObjectByName(NAME_None);
						if (NetworkPhysicsComponent) {
							NetworkPhysicsComponent->CreateDataHistory<FNetInputPhysicsPawn, FNetStatePhysicsPawn>(PhysicsPawnAsync);
						}
					}
				}
			}
		}
	}
}

// Called every frame
void AMyPhysicsPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PhysicsPawnAsync) {
		if (IsLocallyControlled()) {
			if (FAsyncInputPhysicsPawn* AsyncInput = PhysicsPawnAsync->GetProducerInputData_External()) {
				AsyncInput->MovementInput = ForwardInput_External - BackwardInput_External;
				AsyncInput->SteeringInput = SteeringInput_External;
				AsyncInput->YawInput = YawInput_External;
			}
		}
	}
}

// Called to bind functionality to input
void AMyPhysicsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyPhysicsPawn::SetForwardInput(const float InForwardInput)
{
	ForwardInput_External = FMath::Clamp(InForwardInput, 0.0f, 1.0f);
}

void AMyPhysicsPawn::SetBackwardInput(const float InBackwardInput)
{
	BackwardInput_External = FMath::Clamp(InBackwardInput, 0.0f, 1.0f);
}

void AMyPhysicsPawn::SetSteeringInput(const float InSteeringInput)
{
	SteeringInput_External = FMath::Clamp(InSteeringInput, -1.0f, 1.0f);
}

void AMyPhysicsPawn::SetYawInput(const float InYawInput)
{
	YawInput_External = FMath::Clamp(InYawInput, -1.0f, 1.0f);
}

#pragma region NetInput;

void FNetInputPhysicsPawn::InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha)
{
	const FNetInputPhysicsPawn& MinInput = static_cast<const FNetInputPhysicsPawn&>(MinData);
	const FNetInputPhysicsPawn& MaxInput = static_cast<const FNetInputPhysicsPawn&>(MaxData);
	MovementInput = FMath::Lerp(MinInput.MovementInput, MaxInput.MovementInput, LerpAlpha);
	SteeringInput = FMath::Lerp(MinInput.SteeringInput, MaxInput.SteeringInput, LerpAlpha);
	YawInput = FMath::Lerp(MinInput.SteeringInput, MaxInput.YawInput, LerpAlpha);
}

void FNetInputPhysicsPawn::MergeData(const FNetworkPhysicsPayload& FromData)
{
	const FNetInputPhysicsPawn& FromInput = static_cast<const FNetInputPhysicsPawn&>(FromData);
	MovementInput = FMath::Max(MovementInput, FromInput.MovementInput);
	SteeringInput = FMath::Max(SteeringInput, FromInput.SteeringInput);
	YawInput = FMath::Max(YawInput, FromInput.YawInput);
}

void FNetInputPhysicsPawn::DecayData(float DecayAmount)
{
	MovementInput = MovementInput * (1.0f - DecayAmount);
	SteeringInput = SteeringInput * (1.0f - DecayAmount);
	YawInput = YawInput * (1.0f - DecayAmount);
}

bool FNetInputPhysicsPawn::CompareData(const FNetworkPhysicsPayload& PredictedData) const
{
	const FNetInputPhysicsPawn& PredictedInput = static_cast<const FNetInputPhysicsPawn&>(PredictedData);
	bool bHasDiff = false;
	bHasDiff |= MovementInput != PredictedInput.MovementInput;
	bHasDiff |= SteeringInput != PredictedInput.SteeringInput;
	return (bHasDiff == false);
}


#pragma endregion NetInput;

#pragma region AsyncInput

void FPhysicsPawnAsync::OnPostInitialize_Internal()
{
	if (PhysicsObject) {
		Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
		if (Chaos::FPBDRigidParticleHandle* ParticleHandle = Interface.GetRigidParticle(PhysicsObject)) {
			ParticleHandle->SetSleepType(Chaos::ESleepType::NeverSleep);
		}
	}
}

void FPhysicsPawnAsync::ProcessInputs_Internal(int32 PhysicsStep)
{
	const FAsyncInputPhysicsPawn* AsyncInput = GetConsumerInput_Internal();
	if (AsyncInput == nullptr) {
		return;
	}
	Chaos::FPhysicsSolverBase* BaseSolver = GetSolver();
	if (!BaseSolver || BaseSolver->IsResimming())
	{
		return;
	}
	SetMovementInput_Internal(AsyncInput->MovementInput);
	SetSteeringInput_Internal(AsyncInput->SteeringInput);
	SetYawInput_Internal(AsyncInput->YawInput);
}

void FPhysicsPawnAsync::OnPreSimulate_Internal()
{
	if (!PhysicsObject) {
		return;
	}
	Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
	Chaos::FPBDRigidParticleHandle* ParticleHandle = Interface.GetRigidParticle(PhysicsObject);
	if (ParticleHandle == nullptr) {
		return;
	}
	if (const FAsyncInputPhysicsPawn* AsyncInput = GetConsumerInput_Internal())
	{
		SetMovementInput_Internal(AsyncInput->MovementInput);
		SetSteeringInput_Internal(AsyncInput->SteeringInput);
		SetYawInput_Internal(AsyncInput->YawInput);
	}

	constexpr float ForceMultiplier = 300000.0f;
	const float InputLinearMovementForce = MovementInput_Internal * ForceMultiplier;
	const float InputLinearSteeringForce = SteeringInput_Internal * ForceMultiplier;
	const float InputAngularMovementForce = MovementInput_Internal * ForceMultiplier * -1.0f;
	const float InputAngularSteeringForce = SteeringInput_Internal * ForceMultiplier;
	const float InputAngularYawForce = YawInput_Internal * ForceMultiplier * 2.0f;
	Chaos::FVec3 LinearMovement = Chaos::FVec3(InputLinearMovementForce, InputLinearSteeringForce, 0.0f);
	Chaos::FVec3 AngularMovement = Chaos::FVec3(0.0f, 0.0f, InputAngularYawForce);
	//Custom Gravity Test
	ParticleHandle->AddForce({ 0,0,-0.1f * ForceMultiplier }, true);

	if (LinearMovement.SizeSquared() > UE_SMALL_NUMBER) {
		ParticleHandle->AddForce(LinearMovement, true);
	}

	constexpr float MaxVelocity = 3000; // magic number max velocity; for testing

	if (ParticleHandle->GetV().Length() > MaxVelocity) { //naive speed limiting for test;
		FVector VClamped = ParticleHandle->GetV().GetSafeNormal() * MaxVelocity;
		ParticleHandle->SetV(VClamped);
	}

	if (AngularMovement.SizeSquared() > UE_SMALL_NUMBER) {

		ParticleHandle->AddTorque(AngularMovement, true);
	}

}

void FPhysicsPawnAsync::OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject)
{
	if (PhysicsObject == InPhysicsObject) {
		PhysicsObject = nullptr;
	}
}

void FPhysicsPawnAsync::BuildInput_Internal(FNetInputPhysicsPawn& Input) const
{
	Input.MovementInput = MovementInput_Internal;
	Input.SteeringInput = SteeringInput_Internal;	
	Input.YawInput = YawInput_Internal;
}

void FPhysicsPawnAsync::ValidateInput_Internal(FNetInputPhysicsPawn& Input) const
{
	Input.MovementInput = FMath::Clamp(Input.MovementInput, -1.0f, 1.0f);
	Input.SteeringInput = FMath::Clamp(Input.SteeringInput, -1.0f, 1.0f);
	Input.YawInput = FMath::Clamp(Input.YawInput, -1.0f, 1.0f);
}

void FPhysicsPawnAsync::ApplyInput_Internal(const FNetInputPhysicsPawn& Input)
{
	SetMovementInput_Internal(Input.MovementInput);
	SetSteeringInput_Internal(Input.SteeringInput);
	SetYawInput_Internal(Input.YawInput);
}

void FPhysicsPawnAsync::BuildState_Internal(FNetStatePhysicsPawn& State) const
{
}

void FPhysicsPawnAsync::ApplyState_Internal(const FNetStatePhysicsPawn& State)
{
}

#pragma endregion AsyncInput


