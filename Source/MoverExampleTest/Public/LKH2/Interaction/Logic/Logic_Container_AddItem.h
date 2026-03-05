// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Container_AddItem.generated.h"

class AItemBase;
class UInteractablePropertyComponent;

/**
 * Container(접시, 냄비 등)에 아이템을 담는 로직 모듈.
 * Container의 모듈 배열에 등록됩니다.
 * 
 * Context.TargetActor를 통해 "플레이어가 보고 있는 대상"을 인식합니다.
 * 대상이 ItemBase이면 직접 담기, Workstation이면 작업대의 아이템을 가져와 담기.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Logic : Container Add Item"))
class MOVEREXAMPLETEST_API ULogic_Container_AddItem : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 담을 수 있는 아이템 태그 (비어있으면 모두 허용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FGameplayTagContainer AcceptedItemTags;

	// 담을 수 없는 아이템 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FGameplayTagContainer RejectedItemTags;

	// Container 가용한 슬롯 태그 목록 (에디터에서 직접 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	TArray<FGameplayTag> SlotTags;

	// 작업대의 아이템을 가져올 때 사용할 슬롯 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FGameplayTag WorkstationSlotTag;

public:
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;
	virtual bool PerformInteraction(const FInteractionContext &Context) override;

private:
	/** 대상 아이템이 필터를 통과하는지 확인 */
	bool CanAcceptItem(AItemBase* Item) const;

	/** Container에서 빈 슬롯 태그를 찾아 반환 (없으면 빈 태그) */
	FGameplayTag FindAvailableSlot(UInteractablePropertyComponent* ContainerProperty) const;
};
