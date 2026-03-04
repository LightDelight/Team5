// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Holding/Logic_Holding_SpillCleanup.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "LKH2/LKH2GameplayTags.h"

ULogic_Holding_SpillCleanup::ULogic_Holding_SpillCleanup()
{
}

// ─────────────────────────────────────────────────────────────────────────────
// PreInteractCheck
// ─────────────────────────────────────────────────────────────────────────────
bool ULogic_Holding_SpillCleanup::PreInteractCheck(const FInteractionContext& Context)
{
    // ─── 경로 1: SpillCleanup Intent ─ UID → 블랙보드 누적 ───
    if (Context.InteractionTag.MatchesTagExact(FLKH2GameplayTags::Get().Intent_Cart_SpillCleanup))
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] UID 등록 시도: %s"),
            *Context.ItemUID.ToString());

        if (!Context.ItemUID.IsValid() || !SpilledItemsStatTag.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Logic_Holding_SpillCleanup] UID 등록 실패: ItemUID=%s, StatTag=%s"),
                Context.ItemUID.IsValid() ? TEXT("유효") : TEXT("무효"),
                SpilledItemsStatTag.IsValid() ? TEXT("유효") : TEXT("무효(미설정)"));
            return false;
        }

        // ContextComp는 InteractableComponent.OnInteract에서 TargetActor 기준으로 채워줌
        ULogicContextComponent* ContextComp =
            Cast<ULogicContextComponent>(Context.ContextComp);
        if (!ContextComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Logic_Holding_SpillCleanup] ContextComp 없음"));
            return false;
        }

        UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();
        if (!ItemManager) return false;

        AItemBase* SpilledActor = ItemManager->GetItemActor(Context.ItemUID);
        if (!SpilledActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Logic_Holding_SpillCleanup] UID %s에 해당하는 아이템 없음"),
                *Context.ItemUID.ToString());
            return false;
        }

        // 기존 ObjectArray 읽기
        TArray<UObject*> ExistingArray;
        if (const FItemStatValue* Existing = ContextComp->FindStat(SpilledItemsStatTag))
        {
            for (const TObjectPtr<UObject>& ObjPtr : Existing->ObjectArrayValue)
            {
                if (ObjPtr.Get()) ExistingArray.Add(ObjPtr.Get());
            }
        }

        ExistingArray.AddUnique(SpilledActor);
        ContextComp->SetStat(SpilledItemsStatTag, FItemStatValue::MakeObjectArray(ExistingArray));

        UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] UID 등록 완료: %s (%s) → 총 %d개"),
            *SpilledActor->GetName(), *Context.ItemUID.ToString(), ExistingArray.Num());

        // UID 등록은 PerformInteraction으로 진행하지 않음
        return false;
    }

    // ─── 경로 2: 일반 Interact → 부모(HoldingStampLogicModuleBase)에 위임 ───
    return Super::PreInteractCheck(Context);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnStampCompleted_Implementation
// ─────────────────────────────────────────────────────────────────────────────
void ULogic_Holding_SpillCleanup::OnStampCompleted_Implementation(const FInteractionContext& Context)
{
    UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] ── 홀딩 완료 → 쏟아진 아이템 청소 시작 ──"));

    ULogicContextComponent* ContextComp =
        Context.TargetActor
            ? Context.TargetActor->FindComponentByClass<ULogicContextComponent>()
            : nullptr;
    UItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UItemManagerSubsystem>();

    if (!ContextComp || !ItemManager || !SpilledItemsStatTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Logic_Holding_SpillCleanup] 청소 실패: ContextComp=%s, ItemManager=%s, StatTag=%s"),
            ContextComp ? TEXT("유효") : TEXT("nullptr"),
            ItemManager ? TEXT("유효") : TEXT("nullptr"),
            SpilledItemsStatTag.IsValid() ? TEXT("유효") : TEXT("무효(미설정)"));
        return;
    }

    const FItemStatValue* SpilledStat = ContextComp->FindStat(SpilledItemsStatTag);
    if (!SpilledStat)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Logic_Holding_SpillCleanup] 블랙보드에 SpilledItems 없음 (태그: %s)"),
            *SpilledItemsStatTag.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] 파괴 대상 아이템 %d개"),
            SpilledStat->ObjectArrayValue.Num());

        int32 DestroyedCount = 0;
        for (const TObjectPtr<UObject>& ObjPtr : SpilledStat->ObjectArrayValue)
        {
            AItemBase* Item = Cast<AItemBase>(ObjPtr.Get());
            if (!Item)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Logic_Holding_SpillCleanup]   아이템 포인터 무효 (이미 파괴됨)"));
                continue;
            }
            UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup]   파괴: %s (UID: %s)"),
                *Item->GetName(), *Item->GetInstanceId().ToString());
            ItemManager->DestroyItem(Item->GetInstanceId());
            DestroyedCount++;
        }
        UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] %d개 파괴 완료"), DestroyedCount);
    }

    // 마커(TargetActor) 파괴
    if (AActor* Marker = Context.TargetActor)
    {
        UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] 마커 파괴: %s"), *Marker->GetName());
        Marker->Destroy();
    }

    UE_LOG(LogTemp, Log, TEXT("[Logic_Holding_SpillCleanup] ── 청소 완료 ──"));
}
