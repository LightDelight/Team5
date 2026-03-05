#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

class USceneComponent;

/**
 * 내부에 부착된 UItemSpawnPointComponent 들을 탐색하여,
 * 각각 지정된 태그의 아이템을 ItemManagerSubsystem을 통해 소환합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API AItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemSpawner();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<USceneComponent> DefaultRoot;

	/** BeginPlay 시점에 자동으로 모든 아이템을 스폰할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	/** 하위의 모든 ItemSpawnPointComponent를 찾아서 아이템을 소환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAllItems();
};
