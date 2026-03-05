#include "LKH2/Core/StageGameState.h"
#include "Net/UnrealNetwork.h"

AStageGameState::AStageGameState()
{
	bIsStageActive = false;
	bReplicates = true;
}

void AStageGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStageGameState, TargetItems);
	DOREPLIFETIME(AStageGameState, CollectedItems);
	DOREPLIFETIME(AStageGameState, bIsStageActive);
	DOREPLIFETIME(AStageGameState, RemainingTime);
}

void AStageGameState::OnRep_TargetItems()
{
	OnTargetItemsUpdated.Broadcast();
}

void AStageGameState::OnRep_CollectedItems()
{
	OnCollectedItemsUpdated.Broadcast();
}

void AStageGameState::OnRep_IsStageActive()
{
	OnStageStateChanged.Broadcast();
}

void AStageGameState::SetTargetItems(const TArray<FGameplayTag>& InTargetItems)
{
	if (HasAuthority())
	{
		TargetItems = InTargetItems;
		OnRep_TargetItems(); // 서버 로컬 처리
	}
}

void AStageGameState::SetCollectedItems(const TArray<FGameplayTag>& InCollectedItems)
{
	if (HasAuthority())
	{
		CollectedItems = InCollectedItems;
		OnRep_CollectedItems(); // 서버 로컬 처리
	}
}

void AStageGameState::SetStageActive(bool bActive)
{
	if (HasAuthority() && bIsStageActive != bActive)
	{
		bIsStageActive = bActive;
		OnRep_IsStageActive(); // 서버 로컬 처리
	}
}

void AStageGameState::OnRep_RemainingTime()
{
	OnTimeUpdated.Broadcast(RemainingTime);
}

void AStageGameState::SetRemainingTime(int32 InTime)
{
	if (HasAuthority() && RemainingTime != InTime)
	{
		RemainingTime = InTime;
		OnRep_RemainingTime();
	}
}

bool AStageGameState::AreAllTargetItemsCollected() const
{
	if (TargetItems.IsEmpty()) return false;

	// 목표 아이템 수량이 여러개일 수도 있으므로 각 태그별로 개수를 센 뒤 비교
	TMap<FGameplayTag, int32> RequiredCounts;
	for (const FGameplayTag& Tag : TargetItems)
	{
		RequiredCounts.FindOrAdd(Tag)++;
	}

	TMap<FGameplayTag, int32> CurrentCounts;
	for (const FGameplayTag& Tag : CollectedItems)
	{
		CurrentCounts.FindOrAdd(Tag)++;
	}

	for (const auto& Pair : RequiredCounts)
	{
		int32 Needed = Pair.Value;
		int32 Have = CurrentCounts.Contains(Pair.Key) ? CurrentCounts[Pair.Key] : 0;
		if (Have < Needed) return false;
	}

	return true;
}
