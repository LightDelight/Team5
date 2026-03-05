#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "StageGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetItemsUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectedItemsUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdatedSignature, int32, NewTime);

/**
 * 게임의 스테이지 전반적인 데이터(목표 아이템, 현재 수집된 아이템, 진행 여부, 제한 시간)를 보관하고
 * 모든 클라이언트에 동기화합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AStageGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AStageGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ------------ 목표 아이템 목록 ------------
	
	/** 이번 스테이지의 목표 아이템 태그 목록 */
	UPROPERTY(ReplicatedUsing=OnRep_TargetItems, BlueprintReadOnly, Category="Stage")
	TArray<FGameplayTag> TargetItems;

	UFUNCTION()
	void OnRep_TargetItems();

	UPROPERTY(BlueprintAssignable, Category="Stage|Events")
	FOnTargetItemsUpdatedSignature OnTargetItemsUpdated;


	// ------------ 담긴 아이템 목록 ------------

	/** 카트에 담겨서 조건을 충족시킨/또는 수집 완료된 아이템 태그 목록 */
	UPROPERTY(ReplicatedUsing=OnRep_CollectedItems, BlueprintReadOnly, Category="Stage")
	TArray<FGameplayTag> CollectedItems;

	UFUNCTION()
	void OnRep_CollectedItems();

	UPROPERTY(BlueprintAssignable, Category="Stage|Events")
	FOnCollectedItemsUpdatedSignature OnCollectedItemsUpdated;

	// ------------ 진행 상태 ------------
	
	/** 스테이지 진행 중(true)인지, 종료/대기중(false)인지 여부 */
	UPROPERTY(ReplicatedUsing=OnRep_IsStageActive, BlueprintReadOnly, Category="Stage")
	bool bIsStageActive;

	UFUNCTION()
	void OnRep_IsStageActive();

	UPROPERTY(BlueprintAssignable, Category="Stage|Events")
	FOnStageStateChangedSignature OnStageStateChanged;

	// ------------ 제한 시간 ------------

	/** 스테이지 남은 시간 (초 단위) */
	UPROPERTY(ReplicatedUsing=OnRep_RemainingTime, BlueprintReadOnly, Category="Stage|Time")
	int32 RemainingTime;

	UFUNCTION()
	void OnRep_RemainingTime();

	UPROPERTY(BlueprintAssignable, Category="Stage|Events")
	FOnTimeUpdatedSignature OnTimeUpdated;

	// ------------ 권한(Authority) 전용 상태 설정 함수 ------------

	void SetTargetItems(const TArray<FGameplayTag>& InTargetItems);
	void SetCollectedItems(const TArray<FGameplayTag>& InCollectedItems);
	void SetStageActive(bool bActive);
	void SetRemainingTime(int32 InTime);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stage")
	bool AreAllTargetItemsCollected() const;
};
