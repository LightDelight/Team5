// Fill out your copyright notice in the Description page of Project Settings.

#include "CartBase.h"
#include "MyCommonMacros.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "Chaos/PhysicsObjectInternalInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "DrawDebugHelpers.h"


// Sets default values
ACartBase::ACartBase()
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
void ACartBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACartBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACartBase::PawnClientRestart()
{
	Super::PawnClientRestart();
	auto* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) {
		return;
	}
	auto* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem) {
		return;
	}
	InputSubsystem->AddMappingContext(DefaultInputMapping, 0);

}


void ACartBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UPrimitiveComponent* RootSimulatedComponent = Cast<UPrimitiveComponent>(GetRootComponent())) {
		if (UWorld* World = GetWorld()) {
			if (FPhysScene* PhysScene = World->GetPhysicsScene()) {
				if (Chaos::FPhysicsSolver* Solver = PhysScene->GetSolver()) {
					PhysicsCart = Solver->CreateAndRegisterSimCallbackObject_External<FPhysicsCart>();
					if (ensure(PhysicsCart)) {
						PhysicsCart->PhysicsObject = RootSimulatedComponent->GetPhysicsObjectByName(NAME_None);
						if (NetworkPhysicsComponent) {
							NetworkPhysicsComponent->CreateDataHistory<FNetInputCart, FNetStateCart>(PhysicsCart);
						}
					}
				}
			}
		}
	}
}

// Called every frame
void ACartBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PhysicsCart) {
		if (IsLocallyControlled()) {
			// Waring - client authorized input production, cheating possible.
			// traction test - simple ground contact test
			FHitResult Result;
			constexpr float TraceDistance = -30.0f;
			FVector ActorLocation = GetActorLocation();
			FVector ActorUpVector = GetActorUpVector();
			FVector TargetLocation = ActorLocation + (ActorUpVector * TraceDistance);
			DrawDebugLine(GetWorld(), ActorLocation, TargetLocation, FColor::Red, false, 3.0f);
			FCollisionObjectQueryParams QueryParams;
			QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
			QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			bool Hit = GetWorld()->LineTraceSingleByObjectType( // traction test, simple from root bone. 
				Result
				, ActorLocation
				, TargetLocation
				, QueryParams
				, Params
			);
			if (!Hit) {
				MovementInput_External = MovementInput_External * TractionLost_Multiplier;
				SteeringInput_External = SteeringInput_External * TractionLost_Multiplier;

			}
			if (FAsyncInputCart* AysncInput = PhysicsCart->GetProducerInputData_External()) {
				AysncInput->MovementInput = MovementInput_External;
				AysncInput->SteeringInput = SteeringInput_External;
			}
		}
		else if (!IsPlayerControlled()) {
			PhysicsCart->GetProducerInputData_External();
		}
	}
}

// Called to bind functionality to input
void ACartBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input) {
		Input->BindAction(IA_MovementInput, ETriggerEvent::Triggered, this, &ACartBase::OnMovementInput);
		Input->BindAction(IA_MovementInput, ETriggerEvent::Completed, this, &ACartBase::OnMovementInput);
		Input->BindAction(IA_SteeringInput, ETriggerEvent::Triggered, this, &ACartBase::OnSteeringInput);
		Input->BindAction(IA_SteeringInput, ETriggerEvent::Completed, this, &ACartBase::OnSteeringInput);
	}
}

void ACartBase::UnPossessed()
{
	Super::UnPossessed();
}


void ACartBase::OnMovementInput(const FInputActionInstance& Instance)
{
	FVector Input = Instance.GetValue().Get<FVector>();
	Input.Normalize();
	Input *= MovementInput_Multiplier;
	MovementInput_External = FRotator(0.0f, GetControlRotation().Yaw, 0.0f).RotateVector(Input);
}

void ACartBase::OnSteeringInput(const FInputActionInstance& Instance)
{
	SteeringInput_External = Instance.GetValue().Get<float>() * SteeringInput_Multiplier;
}


#pragma region NetInput

void FNetInputCart::InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha)
{
	const FNetInputCart& MinInput = static_cast<const FNetInputCart&>(MinData);
	const FNetInputCart& MaxInput = static_cast<const FNetInputCart&>(MaxData);
	MovementInput = FMath::Lerp(MinInput.MovementInput, MaxInput.MovementInput, LerpAlpha);
	SteeringInput = FMath::Lerp(MinInput.SteeringInput, MaxInput.SteeringInput, LerpAlpha);
}

void FNetInputCart::MergeData(const FNetworkPhysicsPayload& FromData)
{
	const FNetInputCart& FromInput = static_cast<const FNetInputCart&>(FromData);
	MovementInput = MovementInput + FromInput.MovementInput;
	SteeringInput = SteeringInput + FromInput.SteeringInput;
}

void FNetInputCart::DecayData(float DecayAmount)
{
	MovementInput = MovementInput * (1.0f - DecayAmount);
	SteeringInput = SteeringInput * (1.0f - DecayAmount);
}

bool FNetInputCart::CompareData(const FNetworkPhysicsPayload& PredictedData) const
{
	const FNetInputCart& PredictedInput = static_cast<const FNetInputCart&>(PredictedData);
	bool bHasDiff = false;
	bHasDiff |= MovementInput != PredictedInput.MovementInput;
	bHasDiff |= SteeringInput != PredictedInput.SteeringInput;
	return (bHasDiff == false);
}

#pragma endregion NetInput

#pragma region AsyncInput

void FPhysicsCart::OnPostInitialize_Internal()
{
	if (PhysicsObject) {
		Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
		if (Chaos::FPBDRigidParticleHandle* ParticleHandle = Interface.GetRigidParticle(PhysicsObject)) {
			ParticleHandle->SetSleepType(Chaos::ESleepType::NeverSleep);
		}
	}
}

void FPhysicsCart::ProcessInputs_Internal(int32 PhysicsStep)
{
	const FAsyncInputCart* AsyncInput = GetConsumerInput_Internal();
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
}

void FPhysicsCart::OnPreSimulate_Internal()
{
	if (!PhysicsObject) {
		return;
	}
	Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
	Chaos::FPBDRigidParticleHandle* ParticleHandle = Interface.GetRigidParticle(PhysicsObject);
	if (ParticleHandle == nullptr) {
		return;
	}
	if (const FAsyncInputCart* AsyncInput = GetConsumerInput_Internal())
	{
		SetMovementInput_Internal(AsyncInput->MovementInput);
		SetSteeringInput_Internal(AsyncInput->SteeringInput);
	}

	constexpr float ForceMultiplier = 300000.0f;
	constexpr float AngularForceMultiplier = 3000000.0f;
	const FVector InputMovementForce = MovementInput_Internal * ForceMultiplier;
	float InputSteeringForce = SteeringInput_Internal * AngularForceMultiplier;

	if (InputMovementForce.SizeSquared() > UE_SMALL_NUMBER) {
		ParticleHandle->AddForce(InputMovementForce, true);
	}

	//PRINT(TEXT("%f"), ParticleHandle->GetV().Length());
	if (ParticleHandle->GetV().SquaredLength() < 20000.0f) { // step fucntion, consider smoothing
		// Since we removed wheel for cart this is required for
		// Zero velocity in place rotate friction overcome multiplier; tesing...
		InputSteeringForce *= 3.0f; // magic constant
	}

	constexpr float MaxVelocity = 5000; // magic number max velocity; for testing

	if (ParticleHandle->GetV().Length() > MaxVelocity) { //naive speed limiting for test;
		FVector VClamped = ParticleHandle->GetV().GetSafeNormal() * MaxVelocity;
		ParticleHandle->SetV(VClamped);
	}

	if (FMath::Abs(InputSteeringForce) > UE_SMALL_NUMBER) {

		ParticleHandle->AddTorque({ 0,0,InputSteeringForce }, true);
	}
}

void FPhysicsCart::OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject)
{
	if (PhysicsObject == InPhysicsObject) {
		PhysicsObject = nullptr;
	}
}

void FPhysicsCart::BuildInput_Internal(FNetInputCart& Input) const
{
	Input.MovementInput = MovementInput_Internal;
	Input.SteeringInput = SteeringInput_Internal;
}

void FPhysicsCart::ValidateInput_Internal(FNetInputCart& Input) const
{
	//Input.MovementInput = Input.MovementInput.GetSafeNormal(0.01); // don't validate for now, testing.
	//Input.SteeringInput = FMath::Clamp(Input.SteeringInput, -1.0f, 1.0f);
}

void FPhysicsCart::ApplyInput_Internal(const FNetInputCart& Input)
{
	SetMovementInput_Internal(Input.MovementInput);
	SetSteeringInput_Internal(Input.SteeringInput);
}

void FPhysicsCart::BuildState_Internal(FNetStateCart& State) const
{
}

void FPhysicsCart::ApplyState_Internal(const FNetStateCart& State)
{
}


#pragma endregion AsyncInput



