#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "ItemSpawnPointComponent.generated.h"

/**
 * 아이템을 소환할 위치와 속성을 지정하기 위한 컴포넌트입니다.
 * ItemSpawner 하위에 다수 배치하여 일괄 소환할 수 있습니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UItemSpawnPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UItemSpawnPointComponent();

	/** 이곳에서 소환할 아이템의 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FGameplayTag ItemTag;
};
