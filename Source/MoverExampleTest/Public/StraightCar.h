

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StraightCar.generated.h"

UCLASS()
class MOVEREXAMPLETEST_API AStraightCar : public AActor
{
	GENERATED_BODY()

public:
	AStraightCar();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* CarMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ResetDistance;

private:
	FVector StartLocation;

	
	bool bIsReturning;
};