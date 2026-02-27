#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StaffCharacter.generated.h"

UCLASS()
class MOVEREXAMPLETEST_API AStaffCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AStaffCharacter();
protected:

	virtual void BeginPlay() override;

	
	UFUNCTION()
	void OnEmployeeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category="AI Movement")
	float EmployeeSpeed=300.0f;	

	UPROPERTY(EditAnywhere, Category = "AI Hindrance")
	float PushStrength = 60000.0f;
};
