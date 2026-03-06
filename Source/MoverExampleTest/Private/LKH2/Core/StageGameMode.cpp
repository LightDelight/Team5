#include "LKH2/Core/StageGameMode.h"
#include "LKH2/Core/StageGameState.h"
#include "LKH2/Core/StagePlayerController.h"

AStageGameMode::AStageGameMode()
{
	GameStateClass = AStageGameState::StaticClass();
	PlayerControllerClass = AStagePlayerController::StaticClass();
}

void AStageGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 설정된 디폴트 목표 아이템이 있다면 게임 시작 시 자동 세팅
	if (!DefaultTargetItems.IsEmpty())
	{
		StartStage(DefaultTargetItems);
	}
}

void AStageGameMode::StartStage(const TArray<FGameplayTag>& TargetItems)
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		SGS->SetTargetItems(TargetItems);
		SGS->SetCollectedItems(TArray<FGameplayTag>()); // 초기화
		SGS->SetStageActive(true);
		SGS->SetRemainingTime(StageTimeLimit);

		GetWorldTimerManager().SetTimer(StageTimerHandle, this, &AStageGameMode::OnStageTimerTick, 1.0f, true);
	}
}

void AStageGameMode::AddItemToCart(FGameplayTag AddedItemTag)
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		UE_LOG(LogTemp, Log, TEXT("[StageGameMode] AddItemToCart 수신. 태그: %s"), *AddedItemTag.ToString());
		TArray<FGameplayTag> CurrentCollected = SGS->CollectedItems;
		CurrentCollected.Add(AddedItemTag);
		SGS->SetCollectedItems(CurrentCollected);
		UE_LOG(LogTemp, Log, TEXT("[StageGameMode] GameState CollectedItems 갱신 완료. 현재 %d 개체"), CurrentCollected.Num());
	}
}

void AStageGameMode::RemoveItemFromCart(FGameplayTag RemovedItemTag)
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		TArray<FGameplayTag> CurrentCollected = SGS->CollectedItems;
		int32 Index = CurrentCollected.Find(RemovedItemTag);
		if (Index != INDEX_NONE)
		{
			CurrentCollected.RemoveAtSwap(Index);
			SGS->SetCollectedItems(CurrentCollected);
		}
	}
}

void AStageGameMode::OnCartSpilled()
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		// 카트가 쏟아지면 모인 아이템 초기화
		SGS->SetCollectedItems(TArray<FGameplayTag>());
	}
}

void AStageGameMode::OnStageTimerTick()
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		int32 NewTime = SGS->RemainingTime - 1;
		SGS->SetRemainingTime(NewTime);

		// 시간이 다 되면 즉시 실패 처리
		if (NewTime <= 0)
		{
			EndStage(false);
		}
	}
}

void AStageGameMode::OnCrossedFinishLine()
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		if (!SGS->bIsStageActive) return;

		// 목표를 다 달성하지 못했다면 아무 일도 일어나지 않게 차단 (return)
		if (!SGS->AreAllTargetItemsCollected())
		{
			UE_LOG(LogTemp, Log, TEXT("[StageGameMode] 필수 아이템이 모이지 않아 결승선 통과를 무시합니다."));
			return;
		}

		// 성공적으로 통과했을 때만 이벤트 발생
		OnCrossedFinishLineEvent.Broadcast();

		// 다 모인 채로 통과했으므로 성공 처리
		EndStage(true);
	}
}

void AStageGameMode::EndStage(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(StageTimerHandle);

	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		SGS->SetStageActive(false);

		// 각 플레이어 컨트롤러에 결과 화면을 띄우도록 지시
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AStagePlayerController* SPC = Cast<AStagePlayerController>(It->Get()))
			{
				SPC->Client_ShowStageResult(bSuccess);
			}
		}

		// 블루프린트 오버라이드 이벤트 호출 (데이터 저장, 로비 전환 처리 등)
		K2_OnStageCompleted(bSuccess);
	}
}

void AStageGameMode::TravelToLevel(FName LevelName)
{
	if (UWorld* World = GetWorld())
	{
		// 멀티플레이 방 유지를 위해 보통 ?listen을 붙여서 이동하지만, 
		// 단순 맵 이동이라면 기본적으로 다음과 같이 호출합니다.
		// bAbsolute(두번째 인자)가 false면 상대 경로로 처리하고 이전 옵션을 유지하는 성향이 있습니다.
		FString TravelURL = LevelName.ToString();
		World->ServerTravel(TravelURL, true, false);
	}
}

void AStageGameMode::SetDebugTimeLimit()
{
	if (AStageGameState* SGS = GetGameState<AStageGameState>())
	{
		SGS->SetRemainingTime(3);
		UE_LOG(LogTemp, Warning, TEXT("[StageGameMode] 디버그: 남은 시간을 3초로 강제 변경했습니다."));
	}
}
