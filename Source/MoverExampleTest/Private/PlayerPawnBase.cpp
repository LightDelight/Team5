// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "MyCommonMacros.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "Chaos/PhysicsObjectInternalInterface.h"


// Sets default values
APlayerPawnBase::APlayerPawnBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerPawnBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerPawnBase::PawnClientRestart()
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

// Called every frame
void APlayerPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PRINT_FUNCTION_NAME;
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input) {
		Input->BindAction(IA_MovementInput, ETriggerEvent::Triggered, this, &ThisClass::OnMovementInput);
		Input->BindAction(IA_MovementInput, ETriggerEvent::Completed, this, &ThisClass::OnMovementInput);
	}
}

void APlayerPawnBase::OnMovementInput(const FInputActionInstance& Instance)
{
	FVector Input = Instance.GetValue().Get<FVector>();
	Input.Normalize();
	MovementInput_External = FRotator(0.0f, GetControlRotation().Yaw, 0.0f).RotateVector(Input);
}

void APlayerPawnBase::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	OnProduceInput((float)SimTimeMs, InputCmdResult);
}

void APlayerPawnBase::OnProduceInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd)
{
	FCharacterDefaultInputs& CharacterInputs = OutInputCmd.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

	if (GetController() == nullptr)
	{
		if (GetLocalRole() == ENetRole::ROLE_Authority && GetRemoteRole() == ENetRole::ROLE_SimulatedProxy)
		{
			static const FCharacterDefaultInputs DoNothingInput;
			// If we get here, that means this pawn is not currently possessed and we're choosing to provide default do-nothing input
			CharacterInputs = DoNothingInput;
		}

		// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
		return;
	}

	CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, MovementInput_External);
	CharacterInputs.OrientationIntent = GetControlRotation().Vector();
}
/*
 * May use impact location, add torque and impulse, improve repelling. just simple direction only for now
 */
void APlayerPawnBase::AfflictImpulse_Client(AActor* TargetActor)
{
	if (!TargetActor->GetRootComponent()->IsSimulatingPhysics()) return;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) { check(PC);  return; }
	FVector ImpulseDir = TargetActor->GetActorLocation() - GetActorLocation();
	ImpulseDir.Normalize();

	const int32 ServerFrame = PC->GetPhysicsTimestamp().ServerFrame;
	const int32 ClientFrame = PC->GetPhysicsTimestamp().LocalFrame;

	if (!HasAuthority()) {
		AfflictImpulse_Server(ServerFrame, TargetActor, ImpulseDir);
	}
	ImpulseAfflictor(ClientFrame, TargetActor, ImpulseDir);
}

void APlayerPawnBase::AfflictImpulse_Server_Implementation(int32 CorrespondingFrame, AActor* TargetActor, FVector ImpulseDirection)
{
	ImpulseAfflictor(CorrespondingFrame, TargetActor, ImpulseDirection);
}

void APlayerPawnBase::ImpulseAfflictor(int32 CorrespondingFrame, AActor* TargetActor, FVector ImpulseDirection)
{
	if (!IsValid(TargetActor)) { check(IsValid(TargetActor)); return; }

	UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	if (!IsValid(PrimitiveComponent)) { check(IsValid(PrimitiveComponent)); return; }

	UWorld* World = TargetActor->GetWorld();
	if (!IsValid(World)) { check(World); return; }

	FPhysScene* PhysScene = World->GetPhysicsScene();
	if (!PhysScene) { check(PhysScene); return; }

	Chaos::FPhysicsSolver* Solver = PhysScene->GetSolver();
	if (!Solver) { check(Solver); return; }
	Chaos::FConstPhysicsObjectHandle PhysicsObject = PrimitiveComponent->GetPhysicsObjectByName(NAME_None);

	PrimitiveComponent->WakeRigidBody();
	ImpulseDirection *= 1000;

	Solver->EnqueueCommandScheduled_External(CorrespondingFrame, [PhysicsObject,ImpulseDirection] {
		Chaos::FWritePhysicsObjectInterface_Internal Interface = Chaos::FPhysicsObjectInternalInterface::GetWrite();
		/*
		 * @Markus Boberg
		 * About sleeping though,
		 * you should not need to wake it up manually to be able to apply a force.
		 * It should wake up automatically.
		 * But there are sleeping bugs with resimulation currently (which is why I forced it to NeverSleep in the tutorial),
		 * I¡¯m working on fixing the bugs for UE 5.8.
		 */
		if (Chaos::FPBDRigidParticleHandle* Handle = Interface.GetRigidParticle(PhysicsObject)) {
			// never sleep physics or physical material for sleep prevention for now might impact performance.
			Handle->SetLinearImpulseVelocity({ImpulseDirection}, true);
		}

		});



}
