// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
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
	 * 시작 시간과 종료 시간, 그리고 현재 월드 타임을 기반으로 진행도 비율(0.0 ~ 1.0)을 반환합니다.
	 */
	UFUNCTION(BlueprintPure, Category = "UI|Progress")
	float GetProgressRatio() const;

	/**
	 * 프로퍼티 컴포넌트의 액터가 가진 블랙보드(또는 Context)에서 조회할 [시작 월드 시간] 태그
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Progress")
	FGameplayTag StartTimeTag;

	/**
	 * 프로퍼티 컴포넌트의 액터가 가진 블랙보드(또는 Context)에서 조회할 [완료 예상 월드 시간] 태그
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Progress")
	FGameplayTag EndTimeTag;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UI|Progress")
	TWeakObjectPtr<UInteractablePropertyComponent> TargetPropertyComp;
};
