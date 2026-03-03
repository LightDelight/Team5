// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"

/**
 * 프로젝트 전역에서 사용되는 네이티브 게임플레이 태그들을 중앙 관리합니다.
 * 이 태그들은 프로젝트 세팅에 수동 등록하지 않아도 에디터와 코드에서 즉시 사용 가능합니다.
 */
struct FLKH2GameplayTags
{
public:
	FLKH2GameplayTags();
	static const FLKH2GameplayTags& Get() { return Instance; }
	static void Initialize();

	// ─── [Stats] 진행도 및 상태 관련 ───
	FGameplayTag Stat_Common_CurrentProgress;
	FGameplayTag Stat_Common_MaxProgress;

	// ─── [Times] UI 및 시간 계산 관련 ───
	FGameplayTag Time_Common_StartTime;
	FGameplayTag Time_Common_EndTime;

	// ─── [Types] 아이템 타입 관련 ───
	FGameplayTag Type_Item_Container;

	// ─── [Intents] 상호작용 의도 ───
	FGameplayTag Intent_Workstation_ItemAdd;
	FGameplayTag Intent_Workstation_ItemRemove;

private:
	static FLKH2GameplayTags Instance;
};
