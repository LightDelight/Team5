#include "LKH2/Core/UI/StageItemEntryWidget.h"
#include "LKH2/Core/UI/StageHUDWidget.h" // UStageItemEntryData 정의 사용
#include "Components/TextBlock.h"

void UStageItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (UStageItemEntryData* EntryData = Cast<UStageItemEntryData>(ListItemObject))
	{
		// 1. C++ 단에서 텍스트 블록이 바인딩 되어 있다면 자동 채우기 (디버그용, 나중에 제거 가능)
		if (ItemNameText)
		{
			FString DisplayStr = EntryData->ItemTag.GetTagName().ToString();
			if (EntryData->bIsCollected)
			{
				DisplayStr = TEXT("[V] ") + DisplayStr;
			}
			ItemNameText->SetText(FText::FromString(DisplayStr));
		}

		// 2. 블루프린트에서 아이콘 변경 등 커스텀 디자인 처리를 위해 이벤트 호출
		K2_OnItemEntrySet(EntryData->ItemTag, EntryData->bIsCollected);
	}
}
