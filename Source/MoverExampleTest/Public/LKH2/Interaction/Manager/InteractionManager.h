// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InteractionManager.generated.h"

class UInteractorComponent;
class UInteractableCategoryComponent; // 임시: 대상 워크스테이션 식별용 또는 향후 확장용
class AItemBase;

/**
 * 아이템의 물리적 상태(PropertyComponent)와 논리적 상태(ItemManagerSubsystem)를
 * 조율하여 실제 게임 내 "행동(Action)"을 정의하는 조율자 서브시스템.
 * 
 * 아키텍처 원칙에 따라 매니저는 세부 구현을 알지 못하며, 
 * 각 컴포넌트의 헬퍼 함수들을 순서에 맞게 조합하여 사용합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API UInteractionManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ─── 행동(Action) 스켈레톤 API ───

	/**
	 * 플레이어가 아이템을 줍거나 장착할 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteEquip(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToEquip);

	/**
	 * 플레이어가 들고 있는 아이템을 버리거나 내려놓을 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteDrop(UInteractorPropertyComponent* InteractorProperty, AItemBase* CarriedItem, FVector OptionalImpulse = FVector::ZeroVector);

	/**
	 * 플레이어가 들고 있는 아이템을 특정 거치대(워크스테이션 등)에 거치할 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteStore(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag);

	/**
	 * 워크스테이션 등에 거치된 아이템을 회수하여 들 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteRetrieve(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToRetrieve, FGameplayTag SlotTag);

	/**
	 * 두 아이템을 소비하여 새로운 아이템을 생성한 뒤 지정된 슬롯에 보관하는 조합 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteCombine(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* MaterialA, AItemBase* MaterialB, FGameplayTag ResultItemTag, FGameplayTag TargetSlotTag);

	/**
	 * 플레이어가 들고 있는 아이템을 버릴(파괴할) 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteTrash(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToTrash);

	/**
	 * 특정 아이템을 생성하여 플레이어가 즉시 집게 하는 행동 정의 (자판기/뽑기 로직용).
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteVending(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, FGameplayTag ItemToSpawnTag);

	// 향후 확장: 조합된 결과물 생성, 파괴 후 변경 등 다중 액터 상호작용 행동 정의들이 추가될 수 있습니다.
};
