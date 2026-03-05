// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InteractorPropertyComponent.generated.h"

// Forward declaration
class AActor;
class UAnimMontage;

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

	/* ---------------------------------------------------------
	 * Action & Montage 
	 * --------------------------------------------------------- */
public:
	/** 특정 ActionTag가 설정되었을 때 자동으로 재생할 몽타주 매핑 정보 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Action")
	TMap<FGameplayTag, UAnimMontage*> ActionMontageMap;

	/** 플레이어가 현재 수행 중인 상호작용 행동 태그 (예: Action.Chop) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentActionTag, Category = "Interaction|Action")
	FGameplayTag CurrentActionTag;

	UFUNCTION()
	void OnRep_CurrentActionTag(FGameplayTag OldTag);

	/**
	 * 플레이어의 현재 행동 태그를 설정하고, 매핑된 몽타주가 있다면 재생합니다.
	 * 서버에서 호출해야만 Replicated되어 클라이언트에서도 몽타주가 재생됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void SetActionTag(FGameplayTag NewTag);

	/**
	 * 플레이어의 현재 행동 태그를 초기화하고 재생 중인 몽타주를 정지합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ClearActionTag();

	/** 플레이어의 현재 행동 태그 반환 */
	UFUNCTION(BlueprintPure, Category = "Interaction|Action")
	FGameplayTag GetActionTag() const { return CurrentActionTag; }

protected:
	/** 내부용: 태그에 매핑된 몽타주를 스켈레탈 메시에 재생 요청 */
	void PlayMontageForTag(FGameplayTag Tag);

	/** 내부용: 현재 스켈레탈 메시에서 재생 중인 몽타주 정지 요청 */
	void StopCurrentMontage();

	/** 내부적으로 재생 중인 몽타주의 캐싱용 참조 */
	UPROPERTY(Transient)
	UAnimMontage* ActiveMontage = nullptr;

};
