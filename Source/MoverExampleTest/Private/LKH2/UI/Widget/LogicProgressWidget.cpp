// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/UI/Widget/LogicProgressWidget.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"

void ULogicProgressWidget::InitializeProgressWidget(UInteractablePropertyComponent* InPropertyComp)
{
	TargetPropertyComp = InPropertyComp;
}

float ULogicProgressWidget::GetProgressRatio() const
{
	if (!TargetPropertyComp.IsValid())
	{
		return 0.0f;
	}

	AActor* OwnerActor = TargetPropertyComp->GetOwner();
	if (!OwnerActor)
	{
		return 0.0f;
	}

	// 컨텍스트 인터페이스를 구현했는지 확인
	ILogicContextInterface* ContextInterface = Cast<ILogicContextInterface>(OwnerActor);
	if (!ContextInterface)
	{
		return 0.0f;
	}

	float StartTime = 0.0f;
	float EndTime = 0.0f;

	// 시작 시간 가져오기
	if (const FItemStatValue* StartStat = ContextInterface->FindStat(StartTimeTag))
	{
		if (StartStat->Type == EItemStatType::Float) StartTime = StartStat->FloatValue;
		else if (StartStat->Type == EItemStatType::Int) StartTime = StartStat->IntValue;
	}

	// 종료 시간 가져오기
	if (const FItemStatValue* EndStat = ContextInterface->FindStat(EndTimeTag))
	{
		if (EndStat->Type == EItemStatType::Float) EndTime = EndStat->FloatValue;
		else if (EndStat->Type == EItemStatType::Int) EndTime = EndStat->IntValue;
	}

	// 잘못된 시간 범위거나, 아직 시작되지 않은 로직
	if (EndTime <= StartTime || StartTime <= 0.0f)
	{
		return 0.0f;
	}

	// 현재 월드(또는 서버) 타임 구하기
	// (Network 환경이면 GetServerWorldTimeSeconds 등을 고려해야 하지만 우선 게임 월드 타임 사용)
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}
	float CurrentTime = World->GetTimeSeconds();

	return FMath::Clamp((CurrentTime - StartTime) / (EndTime - StartTime), 0.0f, 1.0f);
}
