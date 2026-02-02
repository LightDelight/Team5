// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverExamplesCharacter.h"
#include "MyMoverTest.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyActor,Log,All);

/**
 * 
 */
UCLASS()
class MOVEREXAMPLETEST_API AMyMoverTest : public AMoverExamplesCharacter
{
	GENERATED_BODY()

public:
	
	virtual void OnRep_ReplicatedMovement() override;
	
};
