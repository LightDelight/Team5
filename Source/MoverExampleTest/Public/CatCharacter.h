#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CatCharacter.generated.h"

class UAnimMontage;

UCLASS()
class MOVEREXAMPLETEST_API ACatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACatCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* ThrowMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* SweepMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* ChopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* PickupMontage;

	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ThrowInput();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void SweepInput();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ChopInput();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void PickupInput();

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action")
	void OnThrowImpact();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action")
	void OnSweepImpact();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action")
	void OnChopImpact();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action")
	void OnPickupImpact();
};