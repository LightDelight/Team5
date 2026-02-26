// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractorPropertyComponent.generated.h"

// Forward declaration
class AActor;

/**
 * 플레이어(조작 액터) 수준의 강한 결합을 가진 상호작용 관련 데이터를 관리하는 컴포넌트입니다.
 * (예: 현재 들고 있는 아이템)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractorPropertyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractorPropertyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** 현재 플레이어가 손에 들고 있는 객체입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CarriedActor, Category = "Interaction|Storage")
	TObjectPtr<AActor> CarriedActor;

	UFUNCTION()
	void OnRep_CarriedActor(AActor* OldCarriedActor);

	/**
	 * 들고 있는 객체를 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction|Storage")
	AActor* GetCarriedActor() const;

	/**
	 * 새로운 객체를 손에 듭니다. 기존 아이템은 탈착 처리됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void ForceEquip(AActor* ItemToEquip);

	/**
	 * 들고 있던 객체를 내려놓습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void ForceDrop();
};
