// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Logic_Interactable_Trash.generated.h"

/**
 * 아이템을 폐기(Trash)하는 상호작용 로직 모듈 스켈레톤입니다.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Interactable Logic: Trash"))
class MOVEREXAMPLETEST_API ULogic_Interactable_Trash : public ULogicModuleBase
{
	GENERATED_BODY()

public:
	// 버릴 수 없는 아이템 태그 목록 (예: 퀘스트 아이템 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Trash")
	FGameplayTagContainer RestrictedItemTags;

public:
	// 상호작용 의도(Intent) 확인 및 가능한 상태인지 검사
	virtual bool PreInteractCheck(const FInteractionContext &Context) override;

	// 실제 쓰레기통 비우기/아이템 폐기 등 핵심 로직 수행
	virtual bool PerformInteraction(const FInteractionContext &Context) override;
};
