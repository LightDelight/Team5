// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Manager/ItemManagerSubsystem.h"
#include "Engine/World.h"
#include "LKH2/Data/ItemRegistryData.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"

void UItemManagerSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);

  // 기본 아이템 스폰 클래스를 AItemBase로 설정
  DefaultItemClass = AItemBase::StaticClass();
}

void UItemManagerSubsystem::Deinitialize() {
  // 정리: 활성 아이템 추적 초기화
  ActiveItems.Empty();
  ItemDataRegistry.Empty();
  ItemClassRegistry.Empty();

  Super::Deinitialize();
}

// ─── 레지스트리 관리 ───

void UItemManagerSubsystem::LoadRegistry(UItemRegistryData *RegistryData) {
  if (!RegistryData)
    return;

  // 기본 클래스 갱신
  if (RegistryData->DefaultItemClass) {
    DefaultItemClass = RegistryData->DefaultItemClass;
  }

  // 항목 일괄 등록
  for (const FItemRegistryEntry &Entry : RegistryData->Entries) {
    if (Entry.ItemTag.IsValid() && Entry.ItemData) {
      RegisterItemData(Entry.ItemTag, Entry.ItemData, Entry.ItemClass);
    }
  }
}

void UItemManagerSubsystem::RegisterItemData(
    FGameplayTag ItemTag, UItemData *Data, TSubclassOf<AItemBase> ItemClass) {
  if (!ItemTag.IsValid() || !Data)
    return;

  ItemDataRegistry.Add(ItemTag, Data);

  // 클래스가 지정된 경우에만 등록 (미지정 시 DefaultItemClass 폴백)
  if (ItemClass) {
    ItemClassRegistry.Add(Data, ItemClass);
  }
}

UItemData *UItemManagerSubsystem::FindItemData(FGameplayTag ItemTag) const {
  if (const TObjectPtr<UItemData> *Found = ItemDataRegistry.Find(ItemTag)) {
    return Found->Get();
  }
  return nullptr;
}

// ─── 스폰 API ───

AItemBase *UItemManagerSubsystem::SpawnItem(FGameplayTag ItemTag,
                                            const FTransform &Transform) {
  UItemData *Data = FindItemData(ItemTag);
  if (!Data) {
    UE_LOG(LogTemp, Warning,
           TEXT("ItemManagerSubsystem::SpawnItem - Tag '%s'에 해당하는 "
                "ItemData가 레지스트리에 없습니다."),
           *ItemTag.ToString());
    return nullptr;
  }
  return SpawnItemFromData(Data, Transform);
}

AItemBase *UItemManagerSubsystem::SpawnItemFromData(
    UItemData *Data, const FTransform &Transform,
    TSubclassOf<AItemBase> ClassOverride) {
  UWorld *World = GetWorld();
  if (!World || !Data)
    return nullptr;

  UClass *SpawnClass = ResolveSpawnClass(Data, ClassOverride);
  if (!SpawnClass)
    return nullptr;

  // 표준 스폰 파이프라인: Deferred → DataApply → FinishSpawning
  AItemBase *NewItem =
      World->SpawnActorDeferred<AItemBase>(SpawnClass, Transform);
  if (!NewItem)
    return nullptr;

  NewItem->SetItemDataAndApply(Data);
  NewItem->FinishSpawning(Transform);

  // 활성 아이템 추적 등록
  CleanupStaleEntries();
  ActiveItems.Add(NewItem);

  return NewItem;
}

// ─── 파괴 ───

void UItemManagerSubsystem::DestroyItem(AItemBase *Item) {
  if (!Item)
    return;

  // 추적 목록에서 제거
  ActiveItems.RemoveAll(
      [Item](const TWeakObjectPtr<AItemBase> &WeakItem) {
        return !WeakItem.IsValid() || WeakItem.Get() == Item;
      });

  Item->Destroy();
}

// ─── 내부 헬퍼 ───

UClass *UItemManagerSubsystem::ResolveSpawnClass(
    UItemData *Data, TSubclassOf<AItemBase> ClassOverride) const {
  // 우선순위: Override > Registry > Default
  if (ClassOverride) {
    return ClassOverride.Get();
  }

  if (const TSubclassOf<AItemBase> *Found = ItemClassRegistry.Find(Data)) {
    if (*Found) {
      return Found->Get();
    }
  }

  return DefaultItemClass.Get();
}

void UItemManagerSubsystem::CleanupStaleEntries() {
  ActiveItems.RemoveAll(
      [](const TWeakObjectPtr<AItemBase> &WeakItem) {
        return !WeakItem.IsValid();
      });
}
