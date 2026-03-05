#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "StageHUDWidget.generated.h"

class AStageGameState;

/**
 * 리스트뷰 항목(Entry)으로 전달할 데이터 오브젝트
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UStageItemEntryData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category="Stage|UI")
	FGameplayTag ItemTag;

	/** 해당 아이템이 이미 수집되었는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Stage|UI")
	bool bIsCollected = false;
};

/**
 * 스테이지 진행 상태(목표 아이템 등)를 표시하기 위한 블루프린트 오버라이드용 C++ Base 클래스입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API UStageHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	/** 
	 * TargetItems나 CollectedItems가 갱신될 때마다 호출되어 통합된 하나뿐인 목록의 갱신을 블루프린트에 알립니다. 
	 * @param TargetItems 목표 아이템 전체 목록
	 * @param CollectedItems 지금까지 수집된 아이템 목록
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|UI")
	void K2_OnStageItemsUpdated(const TArray<FGameplayTag>& TargetItems, const TArray<FGameplayTag>& CollectedItems);

	/** 스테이지의 활성화 여부가 바뀔 때 호출됩니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|UI")
	void K2_OnStageStateChanged(bool bIsActive);

	// ------------ 기본 UI 컴포넌트 (선택적) ------------

	/** 스테이지 진행 상태(활성/비활성 등)를 표시할 텍스트 */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Stage|UI")
	TObjectPtr<class UTextBlock> StageStatusText;

	/** 남은 시간을 표시할 텍스트 */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Stage|UI")
	TObjectPtr<class UTextBlock> TimeText;

	// ------------ 필수 UI 컴포넌트 ------------

	/** 목표 아이템 전체 목록(수집 여부 표기됨)을 표시할 리스트 뷰 (항목은 UStageItemEntryData 사용) */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Stage|UI")
	TObjectPtr<class UListView> TargetItemsList;

private:
	// 델리게이트 바인딩 함수 (두 상황 모두 동일한 갱신 함수 호출)
	UFUNCTION()
	void HandleStageItemsUpdated();

	UFUNCTION()
	void HandleStageStateChanged();

	UFUNCTION()
	void HandleTimeUpdated(int32 NewTime);

	TWeakObjectPtr<AStageGameState> CachedGameState;
};
