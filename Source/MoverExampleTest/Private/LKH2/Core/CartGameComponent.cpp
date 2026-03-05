#include "LKH2/Core/CartGameComponent.h"
#include "LKH2/Core/StageGameMode.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "Kismet/GameplayStatics.h"

UCartGameComponent::UCartGameComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCartGameComponent::HandleItemAdded(AItemBase* Item)
{
	if (!Item || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UItemData* ItemData = Item->GetItemData())
	{
		UE_LOG(LogTemp, Log, TEXT("[CartGameComponent] HandleItemAdded: 아이템 '%s' (Tag: %s) 추가 수신"), 
			*Item->GetName(), *ItemData->ItemTag.ToString());

		if (AStageGameMode* SGM = Cast<AStageGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			SGM->AddItemToCart(ItemData->ItemTag);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[CartGameComponent] 예외 발생: 현재 활성화된 GameMode가 AStageGameMode가 아닙니다!"));
		}
	}
}

void UCartGameComponent::HandleItemRemoved(AItemBase* Item)
{
	if (!Item || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UItemData* ItemData = Item->GetItemData())
	{
		if (AStageGameMode* SGM = Cast<AStageGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			SGM->RemoveItemFromCart(ItemData->ItemTag);
		}
	}
}

void UCartGameComponent::HandleCartSpilled()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (AStageGameMode* SGM = Cast<AStageGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		SGM->OnCartSpilled();
	}
}
