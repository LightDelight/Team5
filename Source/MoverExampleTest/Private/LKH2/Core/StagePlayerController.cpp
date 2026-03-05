#include "LKH2/Core/StagePlayerController.h"
#include "LKH2/Core/UI/StageHUDWidget.h"
#include "LKH2/Core/UI/StageResultWidget.h"
#include "Blueprint/UserWidget.h"

AStagePlayerController::AStagePlayerController()
{
}

void AStagePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && StageHUDWidgetClass)
	{
		SpawnedHUDWidget = CreateWidget<UStageHUDWidget>(this, StageHUDWidgetClass);
		if (SpawnedHUDWidget)
		{
			SpawnedHUDWidget->AddToViewport();
		}
	}
}

void AStagePlayerController::Client_ShowStageResult_Implementation(bool bSuccess)
{
	if (IsLocalPlayerController())
	{
		// 1. C++ 기본 처리: 설정된 WBP가 있다면 스폰해서 화면에 띄우고 이벤트 호출
		if (StageResultWidgetClass)
		{
			if (!SpawnedResultWidget)
			{
				SpawnedResultWidget = CreateWidget<UStageResultWidget>(this, StageResultWidgetClass);
				if (SpawnedResultWidget)
				{
					SpawnedResultWidget->AddToViewport(10); // 결과창은 좀 더 위에
				}
			}

			if (SpawnedResultWidget)
			{
				SpawnedResultWidget->ShowResultInternal(bSuccess);
			}
		}

		// 2. 블루프린트 측의 커스텀 추가 처리가 필요할 경우를 대비한 오버라이드용 이벤트
		K2_OnShowStageResult(bSuccess);
	}
}
