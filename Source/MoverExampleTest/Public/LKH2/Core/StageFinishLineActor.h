#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageFinishLineActor.generated.h"

class UBoxComponent;

/**
 * 카트가 통과하면 해당 스테이지의 클리어(결승선 통과) 판정을 내리는 액터입니다.
 * 에디터에서 월드에 배치하고 Box의 크기를 조절하여 사용합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AStageFinishLineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AStageFinishLineActor();

protected:
	virtual void BeginPlay() override;

	/** 결승선 영역 지정 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FinishLine")
	TObjectPtr<UBoxComponent> FinishZone;

	UFUNCTION()
	void OnFinishZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
