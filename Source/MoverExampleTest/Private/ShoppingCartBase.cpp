// Fill out your copyright notice in the Description page of Project Settings.


#include "ShoppingCartBase.h"
#include "Engine/Engine.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "Chaos/PhysicsObjectInternalInterface.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "MyCommonMacros.h"

DEFINE_LOG_CATEGORY(LogPhysicsPawn);
					  
// Sets default values
AShoppingCartBase::AShoppingCartBase()
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
void AShoppingCartBase::BeginPlay()
{
	Super::BeginPlay();
}

void AShoppingCartBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void AShoppingCartBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UPrimitiveComponent* RootSimulatedComponent = Cast<UPrimitiveComponent>(GetRootComponent())) {
		if (UWorld* World = GetWorld()) {
			if (FPhysScene* PhysScene = World->GetPhysicsScene()) {
				if (Chaos::FPhysicsSolver* Solver = PhysScene->GetSolver()) {
					PhysicsShoppingCart = Solver->CreateAndRegisterSimCallbackObject_External<FPhysicsShoppingCart>();
					if (ensure(PhysicsShoppingCart)) {
						PhysicsShoppingCart->PhysicsObject = RootSimulatedComponent->GetPhysicsObjectByName(NAME_None);
						if (NetworkPhysicsComponent) {
							NetworkPhysicsComponent->CreateDataHistory<FNetInputShoppingCart, FNetStateShoppingCart>(PhysicsShoppingCart);
						}
					}
				}
			}
		}
	}
}

// Called every frame
void AShoppingCartBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShoppingCartBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AShoppingCartBase::On_MovementInput(const FInputActionInstance& Instance)
{
	MY_PRT(FString::Printf(TEXT("%s Executing"),*FString(__func__)));
	FVector Input = Instance.GetValue().Get<FVector>();
	FVector RotatedInput = GetActorRotation().RotateVector(Input);
}

void AShoppingCartBase::On_CrabInputInput(const FInputActionInstance& Instance)
{
	MY_PRT(FString::Printf(TEXT("%s Executing"), *FString(__func__)));
}

#pragma region NetInput

void FNetInputShoppingCart::InterpolateData(const FNetworkPhysicsPayload& MinData, const FNetworkPhysicsPayload& MaxData, float LerpAlpha)
{
}

void FNetInputShoppingCart::MergeData(const FNetworkPhysicsPayload& FromData)
{
}

void FNetInputShoppingCart::DecayData(float DecayAmount)
{
}

bool FNetInputShoppingCart::CompareData(const FNetworkPhysicsPayload& PredictedData) const
{
	return false;
}

#pragma endregion NetInput

#pragma region AsyncInput

void FPhysicsShoppingCart::OnPostInitialize_Internal()
{
}

void FPhysicsShoppingCart::ProcessInputs_Internal(int32 PhysicsStep)
{
}

void FPhysicsShoppingCart::OnPreSimulate_Internal()
{
}

void FPhysicsShoppingCart::OnPhysicsObjectUnregistered_Internal(Chaos::FConstPhysicsObjectHandle InPhysicsObject)
{
}

void FPhysicsShoppingCart::BuildInput_Internal(FNetInputShoppingCart& Input) const
{
}

void FPhysicsShoppingCart::ValidateInput_Internal(FNetInputShoppingCart& Input) const
{
}

void FPhysicsShoppingCart::ApplyInput_Internal(const FNetInputShoppingCart& Input)
{
}

void FPhysicsShoppingCart::BuildState_Internal(FNetStateShoppingCart& State) const
{
}

void FPhysicsShoppingCart::ApplyState_Internal(const FNetStateShoppingCart& State)
{
}


#pragma endregion AsyncInput

