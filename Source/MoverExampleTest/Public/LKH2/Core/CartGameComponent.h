#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CartGameComponent.generated.h"

class AItemBase;

/**
 * 카트(Cart)에 부착되어 아이템이 담기거나 쏟아질 때 게임 모드(스테이지 진행 상황)에 알리는 브릿지 역할을 합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCartGameComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCartGameComponent();

	/**
	 * 아이템이 카트에 완전히 담겼을 때 호출합니다. (카트 BP에서 오버랩/상호작용 처리 완료 시점)
	 */
	UFUNCTION(BlueprintCallable, Category="Stage|Cart")
	void HandleItemAdded(AItemBase* Item);

	/**
	 * 아이템이 외부 요인으로 카트에서 제거되었을 때 호출합니다. 
	 */
	UFUNCTION(BlueprintCallable, Category="Stage|Cart")
	void HandleItemRemoved(AItemBase* Item);

	/**
	 * 카트가 전복되어 모든 아이템이 쏟아져 내렸을 때 호출합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Stage|Cart")
	void HandleCartSpilled();
};
