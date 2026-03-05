// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_LogicEvent.generated.h"

/**
 * 범용적인 태그 기반 (GameplayTag) 로직 이벤트를 발생시키는 노티파이입니다.
 * 애니메이션 재생 도중 (예: 도마 썰기 시 칼이 닿는 순간 등)
 * 특정 태그(예: "Event.Montage.Hit")를 브로드캐스트하여 LogicTask가 이를 수신하고 활용하게 합니다.
 */
UCLASS(meta = (DisplayName = "Logic Event Notify"))
class MOVEREXAMPLETEST_API UAnimNotify_LogicEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_LogicEvent();

	/** 이 노티파이가 발생할 때 전달할 범용 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic|Notify")
	FGameplayTag EventTag;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
