#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "StageGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrossedFinishLineDelegate);

/**
 * 게임의 스테이지 전반적인 흐름을 제어합니다.
 * - 카트에 담긴 물건/쏟아진 물건 확인
 * - 결승선 통과 처리
 */
UCLASS()
class MOVEREXAMPLETEST_API AStageGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AStageGameMode();

	/** 결승선을 통과했을 때 발생하는 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="Stage|Events")
	FOnCrossedFinishLineDelegate OnCrossedFinishLineEvent;

	virtual void BeginPlay() override;

	/**
	 * 스테이지를 시작하고 목표 아이템 목록을 퍼블리싱합니다.
	 * 블루프린트에서 스테이지 초기 설정 시 호출합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void StartStage(const TArray<FGameplayTag>& TargetItems);

	/**
	 * 플레이어가 카트에 특정 아이템을 담았음을 게임모드에 알립니다.
	 * (혹은 카트 자체에서 담길 때 이 함수를 호출)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void AddItemToCart(FGameplayTag AddedItemTag);

	/**
	 * 카트에서 특정 아이템이 빠져나갔을 때(쏟아짐 등) 게임모드에 알립니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void RemoveItemFromCart(FGameplayTag RemovedItemTag);

	/**
	 * 카트가 전복되어 모든 아이템이 쏟아졌을 때 호출합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void OnCartSpilled();

	/**
	 * 카트가 결승선을 통과했을 때 호출합니다.
	 * (스테이지 성공/실패 판정 후 결과를 처리)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void OnCrossedFinishLine();

	/**
	 * 세션 내 모든 플레이어를 데리고 새로운 레벨로 이동합니다. (Server Travel)
	 * @param LevelName 이동할 맵(레벨)의 이름
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Flow")
	void TravelToLevel(FName LevelName);

	/**
	 * [디버그용] 스테이지 남은 시간을 3초로 강제 변경합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Stage|Debug")
	void SetDebugTimeLimit();

protected:
	/** 스테이지 기본 제한 시간 (초 단위, 기본 300초 = 5분) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stage|Time")
	int32 StageTimeLimit = 300;

	/** 타이머 핸들 */
	FTimerHandle StageTimerHandle;

	/** 1초마다 호출되어 남은 시간을 줄이는 함수 */
	void OnStageTimerTick();

	/** 스테이지를 종료하고 성공/실패 여부를 알리는 내부 공통 함수 */
	void EndStage(bool bSuccess);
	/**
	 * 게임 시작 시 자동으로 목표로 설정할 아이템 태그 목록
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stage|Flow")
	TArray<FGameplayTag> DefaultTargetItems;
	/**
	 * 성공/실패 시 블루프린트 측(UI 등) 처리를 위임하는 이벤트
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|Events")
	void K2_OnStageCompleted(bool bSuccess);

};
