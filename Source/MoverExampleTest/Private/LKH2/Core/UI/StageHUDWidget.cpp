#include "LKH2/Core/UI/StageHUDWidget.h"
#include "LKH2/Core/StageGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"

void UStageHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AStageGameState* SGS = Cast<AStageGameState>(UGameplayStatics::GetGameState(this)))
	{
		CachedGameState = SGS;
		
		// 델리게이트 바인딩 (두 상황 모두 하나의 함수로 처리)
		SGS->OnTargetItemsUpdated.AddDynamic(this, &UStageHUDWidget::HandleStageItemsUpdated);
		SGS->OnCollectedItemsUpdated.AddDynamic(this, &UStageHUDWidget::HandleStageItemsUpdated);
		SGS->OnStageStateChanged.AddDynamic(this, &UStageHUDWidget::HandleStageStateChanged);
		SGS->OnTimeUpdated.AddDynamic(this, &UStageHUDWidget::HandleTimeUpdated);

		// 초기 데이터 동기화
		HandleStageItemsUpdated();
		HandleStageStateChanged();
		HandleTimeUpdated(SGS->RemainingTime);
	}
}

void UStageHUDWidget::HandleStageItemsUpdated()
{
	if (const AStageGameState* CurrentGameState = GetWorld()->GetGameState<AStageGameState>())
	{
		if (TargetItemsList)
		{
			TargetItemsList->ClearListItems();

			TArray<FGameplayTag> RemainingCollected = CurrentGameState->CollectedItems;

			// Target 항목들을 순회하며 Collected 목록과 매칭 (동일 아이템 여러 개 대응)
			for (const FGameplayTag& TargetTag : CurrentGameState->TargetItems)
			{
				UStageItemEntryData* EntryData = NewObject<UStageItemEntryData>(this);
				EntryData->ItemTag = TargetTag;

				int32 FoundIdx = RemainingCollected.Find(TargetTag);
				if (FoundIdx != INDEX_NONE)
				{
					EntryData->bIsCollected = true;
					RemainingCollected.RemoveAt(FoundIdx); // 수집 체크했으니 남은 Collected에서 제거
				}
				else
				{
					EntryData->bIsCollected = false;
				}

				TargetItemsList->AddItem(EntryData);
			}
		}

		K2_OnStageItemsUpdated(CurrentGameState->TargetItems, CurrentGameState->CollectedItems);
	}
}

void UStageHUDWidget::HandleStageStateChanged()
{
	if (CachedGameState.IsValid())
	{
		if (StageStatusText)
		{
			FString StatusStr = CachedGameState->bIsStageActive ? TEXT("Status: ACTIVE") : TEXT("Status: WAITING");
			StageStatusText->SetText(FText::FromString(StatusStr));
		}

		K2_OnStageStateChanged(CachedGameState->bIsStageActive);
	}
}

void UStageHUDWidget::HandleTimeUpdated(int32 NewTime)
{
	if (TimeText)
	{
		int32 Minutes = FMath::Max(0, NewTime / 60);
		int32 Seconds = FMath::Max(0, NewTime % 60);
		FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TimeText->SetText(FText::FromString(TimeStr));
	}
}
