#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StagePlayerController.generated.h"

class UStageHUDWidget;
class UStageResultWidget;

/**
 * 스테이지 진행 상태 및 결과에 따라 UI를 띄워주는 역할을 수행합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AStagePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AStagePlayerController();

	virtual void BeginPlay() override;

	/**
	 * 서버(GameMode)에서 결승선을 통과했을 때 결과를 각 클라이언트에 띄우기 위해 호출합니다.
	 * @param bSuccess 목표 달성 여부 (true = 성공, false = 실패)
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Stage|UI")
	void Client_ShowStageResult(bool bSuccess);

protected:
	/**
	 * 해당 컨트롤러를 소유한 로컬 클라이언트에서 결과 화면 위젯 생성/표시를 처리하도록
	 * 블루프린트에 구현을 위임합니다. (C++에서 기본 처리하므로 선택적 오버라이드용)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Stage|UI")
	void K2_OnShowStageResult(bool bSuccess);

	// ------------ UI 설정 ------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stage|UI")
	TSubclassOf<UStageHUDWidget> StageHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stage|UI")
	TSubclassOf<UStageResultWidget> StageResultWidgetClass;

	// ------------ 인스턴스 보관 ------------

	UPROPERTY(Transient)
	TObjectPtr<UStageHUDWidget> SpawnedHUDWidget;

	UPROPERTY(Transient)
	TObjectPtr<UStageResultWidget> SpawnedResultWidget;
};
