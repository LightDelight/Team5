#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameplayTagContainer.h"
#include "StageItemEntryWidget.generated.h"

/**
 * StageHUDWidget의 ListView 등에서 각 아이템(UStageItemEntryData)을 화면에 표시하기 위한
 * 엔트리 위젯의 C++ Base 클래스입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API UStageItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
	// IUserObjectListEntry 인터페이스: 리스트에 바인딩된 오브젝트가 들어올 때 호출됨
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * C++에서 추출한 태그와 수집 여부를 블루프린트 단에 알려 디자인(이미지/텍스트/체크마크)을 업데이트하도록 합니다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|UI")
	void K2_OnItemEntrySet(const FGameplayTag& ItemTag, bool bIsCollected);

	// ------------ 필수 UI 컴포넌트 ------------
	
	/** 태그 이름을 표시할 텍스트 블록 */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Stage|UI")
	TObjectPtr<class UTextBlock> ItemNameText;
};
