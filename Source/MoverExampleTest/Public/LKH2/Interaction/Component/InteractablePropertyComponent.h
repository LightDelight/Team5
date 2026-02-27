// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetComponent.h"
#include "InteractablePropertyComponent.generated.h"

// Forward declaration
class AItemBase;

/**
 * 액터 수준의 강한 결합을 가진 상호작용 관련 데이터를 관리하는 컴포넌트입니다.
 * (예: 거치되어 있는 아이템 등)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractablePropertyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractablePropertyComponent();

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void AttachTargetItem(AItemBase* ItemToStore);

	/**
	 * 아이템을 시각적 부착에서 해제합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Storage")
	void DetachTargetItem(AItemBase* ItemToStore);
};
