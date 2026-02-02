// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMoverTest.h"

DEFINE_LOG_CATEGORY(LogMyActor);

// i think original unreal replication updating pawn movement and physics both,
// causing client animation jittering. conflicts with ChaosMover


void AMyMoverTest::OnRep_ReplicatedMovement()
{
	//UE_LOG(LogMyActor, Log, TEXT("%s called here"), *FString(__func__));
	//Super::OnRep_ReplicatedMovement(); // pasting original engine source... testing...

	if (!IsReplicatingMovement())
	{
		return;
	}

	const FRepMovement& LocalRepMovement = GetReplicatedMovement();

	if (RootComponent)
	{

		// NOTE: This is only needed because ClusterUnion has a flag bHasReceivedTransform
		// which does not get updated until the component's transform is directly set.
		// Until that flag is set, its root particle will be in a disabled state and not
		// have any children, therefore replication will be dead in the water.
		RootComponent->OnReceiveReplicatedState(LocalRepMovement.Location, LocalRepMovement.Rotation.Quaternion(), LocalRepMovement.LinearVelocity, LocalRepMovement.AngularVelocity);



		if (LocalRepMovement.bRepPhysics)
		{
			// Sync physics state
#if DO_GUARD_SLOW
			if (!RootComponent->IsSimulatingPhysics())
			{
				UE_LOG(LogNet, Warning, TEXT("IsSimulatingPhysics() returned false during physics replication for %s"), *GetName());
			}


#endif
			// If we are welded we just want the parent's update to move us.
			UPrimitiveComponent* RootPrimComp = Cast<UPrimitiveComponent>(RootComponent);
			if (!RootPrimComp || !RootPrimComp->IsWelded())
			{
				// not calling this cause movement desync it seems innocent...
				PostNetReceivePhysicState();

				// makes smooth animation, but now transformation jittering.
				// so PostNetReceivePhysics cause mover discrepancy?
				// it does not work seems - -desync movments!		
				//PostNetReceiveVelocity(LocalRepMovement.LinearVelocity);
				//PostNetReceiveLocationAndRotation();
				
			}
		}

	}
}
