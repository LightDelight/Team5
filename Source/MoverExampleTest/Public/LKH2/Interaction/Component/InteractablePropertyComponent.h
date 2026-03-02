// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "InteractablePropertyComponent.generated.h"

// Forward declaration
class AItemBase;
class USceneComponent;

/**
 * 액터 수준의 강한 결합을 가진 상호작용 관련 데이터를 관리하는 컴포넌트입니다.
 * (예: 거치되어 있는 아이템 등)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractablePropertyComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractablePropertyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 진행도 등을 표시하기 위해 액터에 부착할 3D 위젯 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|UI")
	TObjectPtr<UWidgetComponent> ProgressWidgetComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** 
	 * 특정 슬롯 태그에 보관 중인 아이템 매핑 
	 * 키: 슬롯 맵핑 태그 (예: Slot.Left, Slot.Right 등)
	 * 값: 현재 거치된 아이템
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Storage")
	TMap<FGameplayTag, TObjectPtr<AItemBase>> StoredItems;

protected:
	/** 아이템 추가 시 발행할 시스템 의도 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Storage|Intent")
	FGameplayTag ItemAddedIntentTag;

	/** 아이템 제거 시 발행할 시스템 의도 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Storage|Intent")
	FGameplayTag ItemRemovedIntentTag;

public:
	/**
	 * 아이템을 특정 슬롯에 보관 시도 (슬롯이 비어있을 때만 성공)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	bool TryStoreItem(FGameplayTag SlotTag, AItemBase* ItemToStore);

	/**
	 * 특정 슬롯에서 아이템을 꺼내서 반환 (가져온 슬롯은 비워집니다)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	AItemBase* RetrieveItem(FGameplayTag SlotTag);

	/**
	 * 특정 슬롯에 보관되어 있는 아이템 참조 읽기 (상태 변동 없음)
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction|Storage")
	AItemBase* GetStoredItem(FGameplayTag SlotTag) const;

	/**
	 * 특정 슬롯에 아이템이 보관되어 있는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction|Storage")
	bool HasItem(FGameplayTag SlotTag) const;

	/**
	 * 아이템을 시각적 부착 설정합니다. (물리적 종속 관계 생성)
	 * @param SnapComponent 지정 시 해당 컴포넌트 위치에 스냅. nullptr이면 소유 액터의 루트에 스냅.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void AttachTargetItem(AItemBase* ItemToStore, USceneComponent* SnapComponent = nullptr);

	/**
	 * 아이템을 시각적 부착에서 해제합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void DetachTargetItem(AItemBase* ItemToStore);

	/**
	 * Step 기반 프로그레스 위젯을 활성화하고 CurrentStepTag/MaxStepTag를 설정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void ShowStepProgressUI(FGameplayTag CurrentStepTag, FGameplayTag MaxStepTag);

	/**
	 * 프로그레스 위젯을 활성화하고 필요한 태그를 셋팅합니다. (시간 기반 전용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void ShowProgressUI(FGameplayTag StartTimeTag, FGameplayTag EndTimeTag);

	/**
	 * 프로그레스 위젯을 비활성화합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void HideProgressUI();
	
	/**
	 * 프로그레스 위젯의 진행도를 고정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void LockProgressUI();

	/**
	 * 프로그레스 위젯의 진행도 고정을 해제합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void UnlockProgressUI();

	/**
	 * 프로그레스 위젯의 진행도 상태를 초기화합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	virtual void ResetProgressUI();

private:
	/**
	 * 리플리케이션: ProgressWidget 표시 여부
	 * OnRep에서 ShowStepProgressUI 또는 HideProgressUI를 호출합니다.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_ProgressVisible)
	bool bProgressVisible = false;

	/** 리플리케이션: Step 현재 태그 */
	UPROPERTY(Replicated)
	FGameplayTag RepCurrentStepTag;

	/** 리플리케이션: Step 최대 태그 */
	UPROPERTY(Replicated)
	FGameplayTag RepMaxStepTag;

	/** 리플리케이션: 실제 현재 스텝 수치 (Blackboard 우회용) */
	UPROPERTY(Replicated)
	float RepCurrentStep = 0.0f;

	/** 리플리케이션: 실제 최대 스텝 수치 (Blackboard 우회용) */
	UPROPERTY(Replicated)
	float RepMaxStep = 0.0f;

	UFUNCTION()
	void OnRep_ProgressVisible();

public:
	/** 서버 → 클라이언트로 복제될 Step 수치를 설정합니다. */
	void SetRepStepValues(float CurrentStep, float MaxStep)
	{
		RepCurrentStep = CurrentStep;
		RepMaxStep = MaxStep;
	}

	float GetRepCurrentStep() const { return RepCurrentStep; }
	float GetRepMaxStep()     const { return RepMaxStep;     }
};

