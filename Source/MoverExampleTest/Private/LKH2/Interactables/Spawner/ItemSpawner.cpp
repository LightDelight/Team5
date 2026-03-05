#include "LKH2/Interactables/Spawner/ItemSpawner.h"
#include "LKH2/Interactables/Spawner/ItemSpawnPointComponent.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "Engine/World.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"

AItemSpawner::AItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	SetRootComponent(DefaultRoot);
}

void AItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (bSpawnOnBeginPlay)
	{
		SpawnAllItems();
	}
}

void AItemSpawner::SpawnAllItems()
{
	if (UWorld* World = GetWorld())
	{
		if (UItemManagerSubsystem* ItemManager = World->GetSubsystem<UItemManagerSubsystem>())
		{
			TArray<UItemSpawnPointComponent*> SpawnPoints;
			GetComponents<UItemSpawnPointComponent>(SpawnPoints);

			for (UItemSpawnPointComponent* SpawnPoint : SpawnPoints)
			{
				if (SpawnPoint && SpawnPoint->ItemTag.IsValid())
				{
					FGuid SpawnedId = ItemManager->SpawnItem(SpawnPoint->ItemTag, SpawnPoint->GetComponentTransform());
					if (SpawnedId.IsValid())
					{
						// 상태를 Display로 갱신 (물리 끄기 등)
						ItemManager->DisplayItem(SpawnedId);

						// 스폰된 아이템을 가져와서 SpawnPoint에 물리적으로 부착
						if (AItemBase* SpawnedItem = ItemManager->GetItemActor(SpawnedId))
						{
							SpawnedItem->AttachToComponent(SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							
							// 네트워크 리플리케이션(수동 부착 동기화) 정보 갱신
							if (UItemStateComponent* StateComp = SpawnedItem->GetStateComponent())
							{
								StateComp->UpdateAttachmentReplication();
							}
						}
					}
				}
			}
		}
	}
}
