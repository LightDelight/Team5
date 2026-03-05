#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageResultWidget.generated.h"

/**
 * 결승선 통과 시 결과창을 띄우기 위한 블루프린트 오버라이드용 C++ Base 클래스입니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API UStageResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/**
	 * 스테이지 결과 애니메이션이나 텍스트를 블루프린트에서 처리하도록 이벤트를 호출합니다.
	 * @param bSuccess 성공 여부
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|UI")
	void K2_ShowResult(bool bSuccess);

	/** C++ 자체적으로 텍스트를 갱신하는 래퍼 함수 */
	void ShowResultInternal(bool bSuccess);

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="Stage|UI")
	TObjectPtr<class UTextBlock> ResultMessageText;
};
