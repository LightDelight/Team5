#include "CartInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h" // UPrimitiveComponent 사용을 위해 필요
#include "Kismet/KismetMathLibrary.h"
#include "SpillManager.h" 
#include "DrawDebugHelpers.h" // 디버그 드로우 기능 사용

UCartInventoryComponent::UCartInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCartInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCartInventoryComponent, CurrentItems);
}

void UCartInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner()->HasAuthority())
    {
        FindItemSockets();
    }
}

void UCartInventoryComponent::FindItemSockets()
{
    ItemSockets.Empty();
    TArray<UActorComponent*> Comps = GetOwner()->GetComponentsByTag(USceneComponent::StaticClass(), SocketTag);
    for (UActorComponent* Comp : Comps)
    {
        USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
        if (SceneComp) ItemSockets.Add(SceneComp);
    }
}

bool UCartInventoryComponent::TryAddItem(AItemBase* Item)
{
    if (!GetOwner()->HasAuthority()) return false;
    if (!IsValid(Item)) return false;
    if (CurrentItems.Contains(Item)) return false;
    if (IsFull()) return false;

    int32 SlotIndex = CurrentItems.Num();
    if (!ItemSockets.IsValidIndex(SlotIndex)) return false;

    USceneComponent* TargetSocket = ItemSockets[SlotIndex];

    // [Collision] 카트와 아이템이 서로 충돌하지 않도록 루트 컴포넌트끼리 무시 설정 (IgnoreActorWhenMoving 사용)
    if (AActor* OwnerActor = GetOwner())
    {
        // 카트의 루트(Primitive) 컴포넌트 가져오기
        if (UPrimitiveComponent* CartRoot = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
        {
            // 두 번째 인자 true = 충돌 무시 켜기
            CartRoot->IgnoreActorWhenMoving(Item, true);
        }

        // 아이템의 루트(Primitive) 컴포넌트 가져오기
        if (UPrimitiveComponent* ItemRoot = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
        {
            ItemRoot->IgnoreActorWhenMoving(OwnerActor, true);
        }
    }

    Item->Server_PickUp(TargetSocket, NAME_None);

    // [Juice] 초기 랜덤 배치 (자연스러움)
    FRotator RandomRot(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    Item->SetActorRelativeRotation(RandomRot);
    FVector RandomOffset = FMath::VRand() * 5.0f;
    RandomOffset.Z = 0;
    Item->AddActorLocalOffset(RandomOffset);

    CurrentItems.Add(Item);
    return true;
}

void UCartInventoryComponent::SpillAllItems(FVector SpillCenterLocation)
{
    if (!GetOwner()->HasAuthority()) return;
    if (CurrentItems.Num() == 0) return;

    // [Debug] 매니저 위치 표시 (빨간색 큰 구체, 5초 유지)
    DrawDebugSphere(GetWorld(), SpillCenterLocation, 50.0f, 12, FColor::Red, false, 5.0f);
    DrawDebugString(GetWorld(), SpillCenterLocation + FVector(0, 0, 100), TEXT("Spill Manager"), nullptr, FColor::White, 5.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASpillManager* Manager = GetWorld()->SpawnActor<ASpillManager>(ASpillManager::StaticClass(), SpillCenterLocation, FRotator::ZeroRotator, SpawnParams);

    if (Manager)
    {
        AActor* OwnerActor = GetOwner();

        for (AItemBase* Item : CurrentItems)
        {
            if (IsValid(Item))
            {
                // [Collision] 충돌 무시 설정 해제 (복구 - IgnoreActorWhenMoving false)
                if (OwnerActor)
                {
                    if (UPrimitiveComponent* CartRoot = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
                    {
                        CartRoot->IgnoreActorWhenMoving(Item, false);
                    }

                    if (UPrimitiveComponent* ItemRoot = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
                    {
                        ItemRoot->IgnoreActorWhenMoving(OwnerActor, false);
                    }
                }

                FVector RandomLandPos = SpillCenterLocation + (FMath::VRand() * FVector(300, 300, 0));

                // [Debug] 아이템 목표 위치 표시 (초록색 구체 + 노란색 연결선, 5초 유지)
                DrawDebugSphere(GetWorld(), RandomLandPos, 30.0f, 8, FColor::Green, false, 5.0f);
                DrawDebugLine(GetWorld(), SpillCenterLocation, RandomLandPos, FColor::Yellow, false, 5.0f);

                Item->Server_Spill(RandomLandPos);
            }
        }
        Manager->RegisterItems(CurrentItems);
    }
    CurrentItems.Empty();
}

bool UCartInventoryComponent::IsFull() const
{
    return CurrentItems.Num() >= ItemSockets.Num();
}

int32 UCartInventoryComponent::GetItemCount() const
{
    return CurrentItems.Num();
}