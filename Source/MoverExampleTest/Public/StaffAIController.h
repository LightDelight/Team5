#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "StaffAIController.generated.h"

UCLASS()
class MOVEREXAMPLETEST_API AStaffAIController : public AAIController
{
    GENERATED_BODY()

public:
    AStaffAIController();
    virtual void Tick(float DeltaTime) override;

protected:
    float ChaseUpdateTimer = 0.0f;
    float ChaseUpdateInterval = 0.2f;
};