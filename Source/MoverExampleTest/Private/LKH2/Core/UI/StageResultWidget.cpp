#include "LKH2/Core/UI/StageResultWidget.h"
#include "Components/TextBlock.h"

void UStageResultWidget::ShowResultInternal(bool bSuccess)
{
	if (ResultMessageText)
	{
		FString Msg = bSuccess ? TEXT("STAGE CLEAR!") : TEXT("STAGE FAILED...");
		ResultMessageText->SetText(FText::FromString(Msg));
	}
	
	// 블루프린트 애니메이션/로우레벨 이벤트 호출
	K2_ShowResult(bSuccess);
}
