// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Manager/ItemManagerSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "LKH2/Carry/Component/CarryComponent.h"
#include "LKH2/Data/ItemRegistryData.h"
#include "LKH2/Item/ItemBase.h"
#include "LKH2/Item/ItemData.h"
#include "LKH2/Item/ItemStateComponent.h"

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

// ─── 상태 전이 API ───

void UItemManagerSubsystem::StoreItem(AItemBase *Item,
                                      USceneComponent *AttachTarget,
                                      UCarryComponent *Carrier) {
  if (!Item || !AttachTarget)
    return;

  // 1. Carrier에서 분리 (들고 있던 경우)
  if (Carrier) {
    Carrier->ForceDrop();
  }

  // 2. 상태 전이 → Stored (OnRep이 물리/콜리전을 자동 처리)
  if (UItemStateComponent *StateComp =
          Item->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Stored);
  }

  // 3. 대상 컴포넌트에 부착
  Item->AttachToComponent(
      AttachTarget,
      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void UItemManagerSubsystem::RetrieveItem(AItemBase *Item,
                                         UCarryComponent *Carrier) {
  if (!Item || !Carrier)
    return;

  // 1. 기존 부착 해제
  Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  // 2. 상태 전이 → Carried (OnRep이 물리/콜리전을 자동 처리)
  if (UItemStateComponent *StateComp =
          Item->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Carried);
  }

  // 3. Carrier에 장착
  Carrier->ForceEquip(Item);
}

void UItemManagerSubsystem::PickUpItem(AItemBase *Item,
                                       USceneComponent *AttachTarget) {
  if (!Item || !AttachTarget)
    return;

  // 1. 상태 전이 → Carried (OnRep이 물리/콜리전을 자동 처리)
  if (UItemStateComponent *StateComp =
          Item->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Carried);
  }

  // 2. 대상에 부착
  Item->AttachToComponent(
      AttachTarget,
      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void UItemManagerSubsystem::DropItem(AItemBase *Item,
                                     const FVector &Impulse,
                                     UCarryComponent *Carrier) {
  if (!Item)
    return;

  // 1. 상태 전이 전 캐리어에서 분리 및 겹침 방지 위치 보정
  if (Carrier) {
    if (AActor *CarrierOwner = Carrier->GetOwner()) {
      FVector SafeLoc = CarrierOwner->GetActorLocation() +
                        CarrierOwner->GetActorForwardVector() * 80.0f;
      SafeLoc.Z = Item->GetActorLocation().Z;
      Item->SetActorLocation(SafeLoc);
    }
    // 캐리어 참조 끊기 (OnRep_CarriedActor 를 통해 클라이언트 디스플레이 갱신)
    Carrier->ForceDrop();
  } else {
    // Carrier 정보가 없으면 단순 분리
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
  }

  // 2. 상태 전이 → Placed (OnRep이 물리 활성화 + 콜리전 복원을 자동 처리)
  if (UItemStateComponent *StateComp =
          Item->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Placed);
  }

  // 3. 던지기 임펄스 (선택적)
  if (!Impulse.IsZero()) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(Item->GetRootComponent())) {
      RootPrim->AddImpulse(Impulse, NAME_None, true);
    }
  }
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

