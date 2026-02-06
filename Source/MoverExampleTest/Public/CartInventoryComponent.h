#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "CartInventoryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCartInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCartInventoryComponent();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<AItemBase*> CurrentItems;

    TArray<USceneComponent*> ItemSockets;

    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    FName SocketTag = TEXT("ItemSlot");

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(AItemBase* Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SpillAllItems(FVector SpillCenterLocation);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsFull() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount() const;

private:
    void FindItemSockets();
};