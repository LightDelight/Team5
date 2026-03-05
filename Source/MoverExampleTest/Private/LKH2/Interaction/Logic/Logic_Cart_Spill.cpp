// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Cart_Spill.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "LKH2/Interactables/Marker/MarkerBase.h"
#include "LKH2/Interactables/Marker/MarkerData.h"
#include "LKH2/LKH2GameplayTags.h"
#include "LKH2/Core/CartGameComponent.h"
#include "Components/PrimitiveComponent.h"

ULogic_Cart_Spill::ULogic_Cart_Spill()
{
}

bool ULogic_Cart_Spill::PreInteractCheck(const FInteractionContext& Context)
{
    // Intent 확인: Intent.Cart.Overturn이어야 처리
    if (!Context.InteractionTag.MatchesTagExact(FLKH2GameplayTags::Get().Intent_Cart_Overturn))
        return false;

    // 카트에 아이템이 있어야 처리 의미가 있음
    UInteractablePropertyComponent* CartProperty =
        Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
    if (!CartProperty || CartProperty->StoredItems.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Spill] PreCheck 실패: CartProperty=%s, StoredItems.IsEmpty=%s"),
            CartProperty ? TEXT("유효") : TEXT("nullptr"),
            (!CartProperty || CartProperty->StoredItems.IsEmpty()) ? TEXT("true") : TEXT("false"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] PreCheck 통과: 카트 내 아이템 %d개 발견"),
        CartProperty->StoredItems.Num());
    return true;
}

bool ULogic_Cart_Spill::PerformInteraction(const FInteractionContext& Context)
{
    UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
    UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
    if (!InteractionManager || !ItemManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[Logic_Cart_Spill] 매니저 없음 (InteractionManager=%s, ItemManager=%s)"),
            InteractionManager ? TEXT("유효") : TEXT("nullptr"),
            ItemManager ? TEXT("유효") : TEXT("nullptr"));
        return false;
    }

    UInteractablePropertyComponent* CartProperty =
        Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
    AActor* CartActor = Context.TargetActor;
    if (!CartProperty || !CartActor)
    {
        UE_LOG(LogTemp, Error, TEXT("[Logic_Cart_Spill] CartProperty 또는 CartActor 없음"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] ── 카트 전복 시작: %s (아이템 %d개) ──"),
        *CartActor->GetName(), CartProperty->StoredItems.Num());

    // ─── 1. 카트 내 모든 아이템 Spill ───
    TArray<AItemBase*> ItemsToSpill;
    for (auto& Pair : CartProperty->StoredItems)
    {
        if (AItemBase* StoredItem = Pair.Value.Get())
            ItemsToSpill.Add(StoredItem);
    }

    TArray<FGuid> SpilledUIDs;

    for (AItemBase* Item : ItemsToSpill)
    {
        if (!Item) continue;

        const FGuid UID = Item->GetInstanceId();
        SpilledUIDs.Add(UID);

        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] 아이템 Spill: %s (UID: %s)"),
            *Item->GetName(), *UID.ToString());

        // Detach
        CartProperty->DetachTargetItem(Item);

        // StoredItems 맵에서 제거
        for (auto It = CartProperty->StoredItems.CreateIterator(); It; ++It)
        {
            if (It.Value().Get() == Item)
            {
                It.RemoveCurrent();
                break;
            }
        }

        // Spill 상태 전환
        ItemManager->SpillItem(UID);

        // 랜덤 임펄스 부여
        if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
        {
            FVector RandomImpulse = FVector(
                FMath::RandRange(-SpillImpulseStrength, SpillImpulseStrength),
                FMath::RandRange(-SpillImpulseStrength, SpillImpulseStrength),
                FMath::RandRange(0.0f, SpillImpulseStrength)
            );
            UE_LOG(LogTemp, Verbose, TEXT("[Logic_Cart_Spill]   임펄스 적용: %s"), *RandomImpulse.ToString());
            Root->AddImpulse(RandomImpulse, NAME_None, true);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] 총 %d개 아이템 Spill 완료"), SpilledUIDs.Num());

    // ─── 2. 정리 마커 스폰 ───
    if (!CleanupMarkerDA)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Spill] CleanupMarkerDA 없음 → 마커 스폰 생략"));
        return true;
    }
    if (SpilledUIDs.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Spill] Spill된 아이템 없음 → 마커 스폰 생략"));
        return true;
    }

    FTransform SpawnTransform = FTransform(
        CartActor->GetActorRotation(),
        CartActor->GetActorLocation() + CartActor->GetActorRotation().RotateVector(MarkerSpawnOffset)
    );

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] 정리 마커 스폰 시도: 위치 %s"),
        *SpawnTransform.GetLocation().ToString());

    AMarkerBase* CleanupMarker =
        InteractionManager->SafeSpawnMarker(CleanupMarkerDA, SpawnTransform);

    if (!CleanupMarker)
    {
        UE_LOG(LogTemp, Error, TEXT("[Logic_Cart_Spill] 마커 스폰 실패 (SafeSpawnMarker 반환 nullptr)"));
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] 마커 스폰 성공: %s"), *CleanupMarker->GetName());

    // ─── 3. 아이템 UID별로 SpillCleanup Intent 전송 ───
    UInteractableComponent* MarkerInteractable =
        CleanupMarker->FindComponentByClass<UInteractableComponent>();
    if (!MarkerInteractable)
    {
        UE_LOG(LogTemp, Error, TEXT("[Logic_Cart_Spill] 마커에 InteractableComponent 없음 → UID 전송 불가"));
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] UID %d개를 마커에 등록 시작"), SpilledUIDs.Num());

    for (const FGuid& UID : SpilledUIDs)
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill]   → UID 전송: %s"), *UID.ToString());

        FInteractionContext UIDContext;
        UIDContext.Interactor = nullptr;
        UIDContext.InteractionTag = FLKH2GameplayTags::Get().Intent_Cart_SpillCleanup;
        UIDContext.ItemUID = UID;
        MarkerInteractable->OnInteract(UIDContext);
    }

    if (UCartGameComponent* CGC = CartActor->FindComponentByClass<UCartGameComponent>())
    {
        CGC->HandleCartSpilled();
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Spill] ── 카트 전복 처리 완료 ──"));
    return true;
}
