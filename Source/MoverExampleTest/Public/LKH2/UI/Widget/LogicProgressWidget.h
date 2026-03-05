// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "LogicProgressWidget.generated.h"

class UInteractablePropertyComponent;

/**
 * InteractablePropertyComponent와 연결되어,
 * ILogicContextInterface를 통해 시작 시간(StartTime)과 종료 시간(EndTime)을 가져와
 * 현재 월드 타임과 비교하여 진행도를 보여주는 위젯입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API ULogicProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 진행도를 갱신할 컴포넌트(주로 PropertyComp)를 등록합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Progress")
	void InitializeProgressWidget(UInteractablePropertyComponent* InPropertyComp);

	/**
	 * 현재 진행도를 고정합니다. (로직이 일시정지될 때 호출)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Progress")
	void LockProgress();

	/**
	 * 고정된 진행도를 해제하고 다시 현재 로직 계산값을 사용합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Progress")
	void UnlockProgress();

	/**
	 * 진행도 관련 상태를 모두 초기화합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Progress")
	void ResetProgress();

	/**
	 * 시작 시간과 종료 시간, 그리고 현재 월드 타임을 기반으로 진행도 비율(0.0 ~ 1.0)을 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "UI|Progress")
	float GetProgressRatio() const;

	/**
	 * 프로퍼티 컴포넌트의 액터가 가진 블랙보드(또는 Context)에서 조회할 [시작 월드 시간] 태그
	 * (이제 TargetPropertyComp->GetUIState()를 통해 가져옵니다)
	 */
	FGameplayTag StartTimeTag;
	FGameplayTag EndTimeTag;
	FGameplayTag CurrentStepTag;
	FGameplayTag MaxStepTag;

protected:
	/**
	 * Blueprint에서 'ProgressBar'라는 이름의 Progress Bar 위젯과 자동 연결됩니다.
	 * 이름이 다르면 컴파일 경고가 발생합니다.
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	/** NativeTick에서 매 프레임 ProgressBar->SetPercent(GetProgressRatio())를 호출합니다. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Progress")
	TWeakObjectPtr<UInteractablePropertyComponent> TargetPropertyComp;

	/** 진행도가 고정되어 있는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Progress")
	bool bIsLocked = false;

	/** 고정된 시점의 진행도 비율 */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Progress")
	float LockedProgress = 0.0f;
};
