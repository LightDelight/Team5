// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Logic/Logic_Cart_Snap.h"
#include "LKH2/Interaction/Manager/InteractionManager.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/LKH2GameplayTags.h"
#include "LKH2/Core/CartGameComponent.h"

ULogic_Cart_Snap::ULogic_Cart_Snap()
{
}

bool ULogic_Cart_Snap::PreInteractCheck(const FInteractionContext& Context)
{
    // Intent 확인: Intent.Cart.ItemOverlap 이어야 처리
    if (!Context.InteractionTag.MatchesTagExact(FLKH2GameplayTags::Get().Intent_Cart_ItemOverlap))
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] Intent 불일치 → 무시. 수신: %s / 기대: %s"),
            *Context.InteractionTag.ToString(),
            *FLKH2GameplayTags::Get().Intent_Cart_ItemOverlap.ToString());
        return false;
    }

    // TargetActor가 AItemBase인지 확인
    AItemBase* Item = Cast<AItemBase>(Context.TargetActor);
    if (!Item)
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] Overlap 대상이 AItemBase가 아님 → 무시: %s"),
            Context.TargetActor ? *Context.TargetActor->GetName() : TEXT("None"));
        return false;
    }

    // Dropped 상태일 때만 스냅 대상 (Spilled/Stored 제외)
    UItemStateComponent* StateComp = Item->GetStateComponent();
    if (!StateComp || StateComp->CurrentState != EItemState::Dropped)
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] %s - 상태가 Dropped이 아님 → 무시"), *Item->GetName());
        return false;
    }

    // 카트 Property에서 보관 아이템 수 확인
    UInteractablePropertyComponent* CartProperty =
        Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
    if (!CartProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Snap] InteractablePropertyComponent가 없음 → 카트 BP에 컴포넌트를 추가하세요!"));
        return false;
    }

    // SlotTags가 비어있으면 스냅 불가
    if (SlotTags.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Snap] SlotTags가 비어있음 → DA에서 슬롯 태그를 설정하세요!"));
        return false;
    }

    int32 EffectiveMax = FMath::Min(MaxItemCount, SlotTags.Num());
    if (CartProperty->StoredItems.Num() >= EffectiveMax)
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] 카트 슬롯 가득 (%d/%d) → 무시"),
            CartProperty->StoredItems.Num(), EffectiveMax);
        return false;
    }

    return true;
}

bool ULogic_Cart_Snap::PerformInteraction(const FInteractionContext& Context)
{
    UInteractionManager* InteractionManager = GetWorld()->GetSubsystem<UInteractionManager>();
    if (!InteractionManager)
        return false;

    UInteractablePropertyComponent* CartProperty =
        Cast<UInteractablePropertyComponent>(Context.InteractablePropertyComp);
    AItemBase* Item = Cast<AItemBase>(Context.TargetActor);
    if (!CartProperty || !Item)
        return false;

    // SlotTags를 순서대로 순회해 비어있는 첫 번째 슬롯에 저장
    for (const FGameplayTag& CandidateSlot : SlotTags)
    {
        if (!CandidateSlot.IsValid())
            continue;

        // 이미 점유된 슬롯이면 다음으로
        if (CartProperty->StoredItems.Contains(CandidateSlot))
            continue;

        bool bSuccess = InteractionManager->SafeStoreWorldItem(CartProperty, Item, CandidateSlot);

        if (bSuccess)
        {
            if (AActor* CartActor = CartProperty->GetOwner())
            {
                if (UCartGameComponent* CGC = CartActor->FindComponentByClass<UCartGameComponent>())
                {
                    UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] %s → CartGameComponent::HandleItemAdded 호출 시작"), *Item->GetName());
                    CGC->HandleItemAdded(Item);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Snap] %s (CartActor)에 UCartGameComponent가 없습니다!"), *CartActor->GetName());
                }
            }
        }

        UE_LOG(LogTemp, Log, TEXT("[Logic_Cart_Snap] %s → 카트 스냅 %s (SlotTag: %s)"),
            *Item->GetName(),
            bSuccess ? TEXT("성공") : TEXT("실패"),
            *CandidateSlot.ToString());

        return bSuccess;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Logic_Cart_Snap] 사용 가능한 슬롯 없음 → 스냅 실패"));
    return false;
}
