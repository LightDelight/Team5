// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interactables/Data/ItemRegistryData.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"

void UItemManagerSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);

  // 기본 아이템 스폰 클래스를 AItemBase로 설정
  DefaultItemClass = AItemBase::StaticClass();
}

void UItemManagerSubsystem::Deinitialize() {
  // 정리: 활성 아이템 추적 초기화
  ActiveItemsMap.Empty();
  ItemDataRegistry.Empty();

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

  // 에러 클래스 및 플래그 갱신
  bUseErrorClassForMissingClass = RegistryData->bUseErrorClassForMissingClass;
  if (RegistryData->ErrorItemClass) {
    ErrorItemClass = RegistryData->ErrorItemClass;
  }

  // 항목 일괄 등록 (데이터 자가 식별 구조)
  for (UItemData *Data : RegistryData->ItemDatas) {
    if (Data && Data->ItemTag.IsValid()) {
      RegisterItemData(Data->ItemTag, Data);
    }
  }
}

void UItemManagerSubsystem::RegisterItemData(
    FGameplayTag ItemTag, UItemData *Data) {
  if (!ItemTag.IsValid() || !Data)
    return;

  ItemDataRegistry.Add(ItemTag, Data);
}

UItemData *UItemManagerSubsystem::FindItemData(FGameplayTag ItemTag) const {
  if (const TObjectPtr<UItemData> *Found = ItemDataRegistry.Find(ItemTag)) {
    return Found->Get();
  }
  return nullptr;
}

// ─── 스폰 API ───

FGuid UItemManagerSubsystem::SpawnItem(FGameplayTag ItemTag,
                                       const FTransform &Transform) {
  UItemData *Data = FindItemData(ItemTag);
  if (!Data) {
    UE_LOG(LogTemp, Warning,
           TEXT("ItemManagerSubsystem::SpawnItem - Tag '%s'에 해당하는 "
                "ItemData가 레지스트리에 없습니다."),
           *ItemTag.ToString());
    return FGuid();
  }
  return SpawnItemFromData(Data, Transform);
}

FGuid UItemManagerSubsystem::SpawnItemFromData(
    UItemData *Data, const FTransform &Transform,
    TSubclassOf<AItemBase> ClassOverride) {
  UWorld *World = GetWorld();
  if (!World || !Data)
    return FGuid();

  UClass *SpawnClass = ResolveSpawnClass(Data, ClassOverride);
  if (!SpawnClass)
    return FGuid();

  // 표준 스폰 파이프라인: Deferred → DataApply → FinishSpawning
  AItemBase *NewItem =
      World->SpawnActorDeferred<AItemBase>(SpawnClass, Transform);
  if (!NewItem)
    return FGuid();

  // 인스턴스 ID 발급 및 설정
  FGuid NewInstanceId = FGuid::NewGuid();
  NewItem->SetInstanceId(NewInstanceId);

  NewItem->SetItemDataAndApply(Data);
  NewItem->FinishSpawning(Transform);

  // 활성 아이템 추적 등록
  CleanupStaleEntries();
  ActiveItemsMap.Add(NewInstanceId, NewItem);

  return NewInstanceId;
}

// ─── 조회 API ───

AItemBase *UItemManagerSubsystem::GetItemActor(const FGuid &InstanceId) const {
  if (!InstanceId.IsValid())
    return nullptr;

  if (const TWeakObjectPtr<AItemBase> *WeakItem = ActiveItemsMap.Find(InstanceId)) {
    return WeakItem->Get();
  }
  return nullptr;
}

// ─── 파괴 ───

void UItemManagerSubsystem::DestroyItem(const FGuid &InstanceId) {
  AItemBase *Item = GetItemActor(InstanceId);
  if (!Item)
    return;

  // 추적 목록에서 제거
  ActiveItemsMap.Remove(InstanceId);

  Item->Destroy();
}

// ─── 상태 전이 API ───

void UItemManagerSubsystem::StoreItem(const FGuid &InstanceId,
                                      USceneComponent *AttachTarget,
                                      UInteractorComponent *Interactor) {
  AItemBase *Item = GetItemActor(InstanceId);
  if (!Item || !AttachTarget)
    return;

  // 1. Interactor에서 분리 (상호작용 중이던 경우)
  if (Interactor) {
    Interactor->ForceDrop();
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

void UItemManagerSubsystem::RetrieveItem(const FGuid &InstanceId,
                                         UInteractorComponent *Interactor) {
  AItemBase *Item = GetItemActor(InstanceId);
  if (!Item || !Interactor)
    return;

  // 1. 기존 부착 해제
  Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  // 2. 상태 전이 → Carried (OnRep이 물리/콜리전을 자동 처리)
  if (UItemStateComponent *StateComp =
          Item->FindComponentByClass<UItemStateComponent>()) {
    StateComp->SetItemState(EItemState::Carried);
  }

  // 3. Interactor에 장착 (피지컬 + 포인터)
  Item->AttachToComponent(
      Interactor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  Interactor->ForceEquip(Item);
}

void UItemManagerSubsystem::PickUpItem(const FGuid &InstanceId,
                                       USceneComponent *AttachTarget,
                                       UInteractorComponent *Interactor) {
  AItemBase *Item = GetItemActor(InstanceId);
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
      
  // 3. Interactor에 장착 (피지컬 + 포인터)
  if (Interactor) {
    Interactor->ForceEquip(Item);
  }
}

void UItemManagerSubsystem::DropItem(const FGuid &InstanceId,
                                     const FVector &Impulse,
                                     UInteractorComponent *Interactor) {
  AItemBase *Item = GetItemActor(InstanceId);
  if (!Item)
    return;

  // 1. 항상 명시적으로 부착 해제 (이게 없으면 위치 보정 후에도 부모에 종속되어 원래 위치로 스냅됨)
  Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  // 1-1. 상태 전이 전 캐리어에서 참조 끊기 및 겹침 방지 위치 보정
  if (Interactor) {
    if (AActor *InteractorOwner = Interactor->GetOwner()) {
      FVector SafeLoc = InteractorOwner->GetActorLocation() +
                        InteractorOwner->GetActorForwardVector() * 80.0f;
      SafeLoc.Z = Item->GetActorLocation().Z;
      Item->SetActorLocation(SafeLoc);
    }
    // 캐리어 참조 끊기 (OnRep_CarriedActor 를 통해 클라이언트 디스플레이 갱신)
    Interactor->ForceDrop();
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
  // 우선순위: Override > Data's EffectiveClass > ErrorClass(if true) > Default
  if (ClassOverride) {
    return ClassOverride.Get();
  }

  if (TSubclassOf<AItemBase> EffectiveClass = Data->GetEffectiveItemClass()) {
    return EffectiveClass.Get();
  }

  if (bUseErrorClassForMissingClass && ErrorItemClass) {
    return ErrorItemClass.Get();
  }

  return DefaultItemClass.Get();
}

void UItemManagerSubsystem::CleanupStaleEntries() {
  for (auto It = ActiveItemsMap.CreateIterator(); It; ++It) {
    if (!It.Value().IsValid()) {
      It.RemoveCurrent();
    }
  }
}

