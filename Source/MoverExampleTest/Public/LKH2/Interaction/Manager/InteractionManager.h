// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
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

	// ─── [NEW] 원자적(Atomic) Safe 래퍼 API ───
	// 로직 모듈(현장 관리자)이 시나리오를 구성할 때 사용하는 최소 단위 기능들입니다.
	// 모든 함수는 내부적으로 유효성 검사 및 자원 조율을 포함하여 "안전하게" 동작합니다.

	/** 특정 태그의 아이템을 월드에 스폰합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	AItemBase* SafeSpawnItem(FGameplayTag ItemTag, const FTransform& SpawnTransform);

	/** 아이템을 안전하게 파괴하고 월드 추적 목록에서 제거합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	void SafeDestroyItem(AItemBase* Item);

	/** 아이템을 워크스테이션 등의 특정 슬롯에 보관 및 부착합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	bool SafeAttachItemToSlot(AItemBase* Item, UInteractablePropertyComponent* TargetProperty, FGameplayTag SlotTag);

	/** 특정 슬롯에서 아이템을 분리하고 보관을 해제합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	AItemBase* SafeDetachItemFromSlot(UInteractablePropertyComponent* TargetProperty, FGameplayTag SlotTag);

	/** 아이템의 인터랙션 진행도 스탯을 안전하게 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	void SafeUpdateItemStat(AItemBase* Item, FGameplayTag StatTag, float NewValue);

	/** 대상(워크스테이션 또는 아이템)의 진행도 UI를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	void SafeUpdateProgressUI(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentTag, FGameplayTag MaxTag, float Current, float Max, FGameplayTag SlotTag = FGameplayTag());

	/** 플레이어가 아이템을 안전하게 줍거나 장착합니다. (논리 + 물리 동시 보장) */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	void SafePickUpItem(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToEquip);

	/** 플레이어가 들고 있는 아이템을 안전하게 내려놓거나 던집니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	void SafeDropItem(UInteractorPropertyComponent* InteractorProperty, AItemBase* CarriedItem, FVector Impulse = FVector::ZeroVector);

	/** 플레이어가 들고 있는 아이템을 특정 슬롯에 안전하게 거치합니다. (손에서 비움) */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	bool SafeStoreHandItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag);

	/** 월드(바닥 등)에 있는 아이템을 특정 슬롯에 안전하게 거치합니다. (플레이어 손에 영향 없음) */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	bool SafeStoreWorldItem(UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag);

	/** 한 저장소의 슬롯에서 다른 저장소의 슬롯으로 아이템을 보관 이동합니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	bool SafeStoreTransferItem(UInteractablePropertyComponent* SourceProperty, FGameplayTag SourceSlot, UInteractablePropertyComponent* TargetProperty, FGameplayTag TargetSlot);

	/** [Deprecated] SafeStoreHandItem을 사용하세요. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe", meta = (DeprecatedFunction, DeprecationMessage = "Use SafeStoreHandItem or SafeStoreWorldItem instead."))
	bool SafeStoreItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToStore, FGameplayTag SlotTag);

	/** 워크스테이션 등의 슬롯에서 아이템을 안전하게 회수하여 플레이어가 듭니다. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Safe")
	bool SafeRetrieveItem(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, AItemBase* ItemToRetrieve, FGameplayTag SlotTag);

	// ─── 행동(Action) 스켈레톤 API (기존 유지, 점진적 이관 예정) ───

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
	 * 특정 워크스테이션(거치대)에 올려진 아이템을 파괴하고, 새로운 결과 아이템을 생성하여 같은 자리에 보관하는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteTransformItem(UInteractablePropertyComponent* TargetProperty, AItemBase* OriginalItem, FGameplayTag ResultItemTag, FGameplayTag TargetSlotTag);

	/**
	 * 플레이어가 들고 있는 아이템을 버릴(파괴할) 때 호출되는 행동 정의.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteTrash(UInteractorPropertyComponent* InteractorProperty, AItemBase* ItemToTrash);

	/**
	 * 지속 입력(Hold)에 대한 진행 상태를 기록하고 UI를 갱신하도록 요청합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action|Holding")
	void StartHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, float RequiredDuration, FGameplayTag SlotTag = FGameplayTag(), FGuid ItemUID = FGuid(), float InitialProgress = 0.0f);

	/**
	 * 지속 입력(Hold)에 대한 진행 상태 보관을 해제하고 UI를 숨기도록 요청합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action|Holding")
	void ClearHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, FGameplayTag SlotTag = FGameplayTag(), FGuid ItemUID = FGuid());

	/**
	 * 진행 중인 홀딩 프로그레스를 고정된 수치(Step)로 변환하여 동결합니다.
	 * SlotTag 또는 TargetActor(Interactor)를 통해 대상을 정할 수 있습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action|Holding")
	void FreezeHoldingProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag StartTimeTag, FGameplayTag EndTimeTag, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, FGameplayTag SlotTag = FGameplayTag(), FGuid ItemUID = FGuid());

	/**
	 * 단계별 진행(Step-based) 수치를 업데이트하고 UI를 갱신합니다.
	 * SlotTag를 전달하면 해당 슬롯의 StoredItem(ItemBase)에 UI를 표시합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action|Step")
	void UpdateStepProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, float CurrentStep, float MaxStep, FGameplayTag SlotTag = FGameplayTag(), FGuid ItemUID = FGuid());

	/**
	 * 단계별 진행(Step-based) 수치를 초기화하고 UI를 숨깁니다.
	 * SlotTag를 전달하면 해당 슬롯의 StoredItem(ItemBase)에서 UI를 숨깁니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action|Step")
	void ClearStepProgress(UInteractablePropertyComponent* TargetProperty, FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag, FGameplayTag SlotTag = FGameplayTag(), FGuid ItemUID = FGuid());

	/**
	 * 특정 아이템을 생성하여 플레이어가 즉시 집게 하는 행동 정의 (자판기/뽑기 로직용).
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void ExecuteVending(UInteractorPropertyComponent* InteractorProperty, UInteractablePropertyComponent* TargetProperty, FGameplayTag ItemToSpawnTag);

	// UI helpers are now managed internally by InteractablePropertyComponent (InternalSetTimerUI, etc.)

	// 향후 확장: 조합된 결과물 생성, 파괴 후 변경 등 다중 액터 상호작용 행동 정의들이 추가될 수 있습니다.
};
