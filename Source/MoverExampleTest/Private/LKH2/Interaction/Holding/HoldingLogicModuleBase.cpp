// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Base/HoldingLogicModuleBase.h"

UHoldingLogicModuleBase::UHoldingLogicModuleBase()
{
	// 기본 홀딩 시간 1초
	RequiredHoldingTime = 1.0f;
}

bool UHoldingLogicModuleBase::ExecuteInteraction(const FInteractionContext &Context)
{
	// 1. 조건 검사 (조상 클래스의 PreInteractCheck 등)
	if (!PreInteractCheck(Context))
	{
		return false;
	}

	// 2. TODO: 실제 Holding 시스템(가칭 InteractionManager의 StartHolding 등)에 Context를 등록
	// 차후 구조에 맞게 구현 추가
	
	// 임시: 즉시 성공 처리 (현재는 틱 처리 환경 미구축)
	// OnHoldingStarted(Context);
	// OnHoldingCompleted(Context);
	// PostInteract(Context, true);

	// 현재 스켈레톤 상태이므로, 부모를 그대로 호출하거나 Holding 시작(진행중) 상태로 둡니다.
	bool bResult = PerformInteraction(Context);
	PostInteract(Context, bResult);
	
	return bResult;
}

void UHoldingLogicModuleBase::OnHoldingStarted(const FInteractionContext &Context)
{
	// 진행바 UI 띄우기 등의 작업
	UE_LOG(LogTemp, Log, TEXT("[HoldingLogic] Holding Started for %f seconds"), RequiredHoldingTime);
}

void UHoldingLogicModuleBase::OnHoldingCompleted(const FInteractionContext &Context)
{
	// 완료 시 실제 필요한 행동(Manager 조합) 요청
	UE_LOG(LogTemp, Log, TEXT("[HoldingLogic] Holding Completed!"));
}

void UHoldingLogicModuleBase::OnHoldingCanceled(const FInteractionContext &Context)
{
	// 진행바 숨기기 등
	UE_LOG(LogTemp, Log, TEXT("[HoldingLogic] Holding Canceled!"));
}
