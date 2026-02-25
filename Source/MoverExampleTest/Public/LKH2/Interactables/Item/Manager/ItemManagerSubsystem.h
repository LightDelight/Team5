// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "ItemManagerSubsystem.generated.h"

class UItemData;
class AItemBase;
class UItemRegistryData;
class UInteractorComponent;

/**
 * 아이템 생성·파괴·추적을 중앙 관리하는 월드 서브시스템.
 *
 * 주요 책임:
 *  - GameplayTag ↔ UItemData ↔ TSubclassOf<AItemBase> 레지스트리
 *  - SpawnActorDeferred → SetItemDataAndApply → FinishSpawning 표준 스폰 파이프라인
 *  - 월드 내 활성 아이템 추적
 *  - 아이템 상태 전이 + 부착/분리의 단일 창구 (StoreItem, RetrieveItem, PickUpItem, DropItem)
 */
UCLASS()
class MOVEREXAMPLETEST_API UItemManagerSubsystem : public UWorldSubsystem {
  GENERATED_BODY()

public:
  // ─── 라이프사이클 ───

  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;

  // ─── 레지스트리 관리 ───

  /**
   * UItemRegistryData 에셋의 내용을 일괄 등록합니다.
   * Initialize에서 자동 호출되거나, 런타임에 추가 등록할 때 사용합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Registry")
  void LoadRegistry(UItemRegistryData *RegistryData);

  /** 단일 항목 등록 */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Registry")
  void RegisterItemData(FGameplayTag ItemTag, UItemData *Data);

  /** Tag로 UItemData 조회 (미등록 시 nullptr) */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ItemManager|Registry")
  UItemData *FindItemData(FGameplayTag ItemTag) const;

  // ─── 스폰 API ───

  /**
   * GameplayTag로 아이템을 스폰합니다.
   * 레지스트리에서 Data/Class를 자동 조회합니다.
   *
   * @return 스폰된 아이템의 고유 인스턴스 ID (실패 시 유효하지 않은 FGuid)
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Spawn")
  FGuid SpawnItem(FGameplayTag ItemTag, const FTransform &Transform);

  /**
   * UItemData를 직접 지정하여 아이템을 스폰합니다. (내부용/특수 케이스)
   *
   * @param ClassOverride  스폰 클래스 직접 지정 (nullptr이면 레지스트리/기본값 사용)
   * @return 스폰된 아이템의 고유 인스턴스 ID
   */
  FGuid SpawnItemFromData(UItemData *Data, const FTransform &Transform,
                          TSubclassOf<AItemBase> ClassOverride = nullptr);

  // ─── 조회 API (내부 로직용/예외처리용) ───

  /**
   * 인스턴스 ID로 월드 내 활성 아이템 액터를 조회합니다.
   * 원칙적으로 외부 통신은 ID로만 이뤄지나, 불가피한 룩업이나 내부 처리를 위해 제공합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Access")
  AItemBase *GetItemActor(const FGuid &InstanceId) const;

  // ─── 파괴 ───

  /** 인스턴스 ID로 아이템을 파괴하고 추적 목록에서 제거합니다. */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Lifecycle")
  void DestroyItem(const FGuid &InstanceId);

  // ─── 상태 전이 API ───

  /**
   * 아이템을 대상 컴포넌트에 거치합니다.
   *
   * @param InstanceId   거치할 아이템의 인스턴스 ID
   * @param AttachTarget 부착 대상 SceneComponent (InteractableComponent 등)
   * @param Interactor   아이템을 들고 있던 InteractorComponent (nullptr이면 ForceDrop 생략)
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|State")
  void StoreItem(const FGuid &InstanceId, USceneComponent *AttachTarget,
                 UInteractorComponent *Interactor = nullptr);

  /**
   * 거치된 아이템을 분리하여 Interactor에게 전달합니다.
   *
   * @param InstanceId 회수할 아이템 인스턴스 ID
   * @param Interactor 아이템을 받을 InteractorComponent
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|State")
  void RetrieveItem(const FGuid &InstanceId, UInteractorComponent *Interactor);

  /**
   * 아이템을 줍기 처리합니다.
   *
   * @param InstanceId   줍을 아이템 인스턴스 ID
   * @param AttachTarget 부착 대상 SceneComponent (InteractorComponent 또는 Actor)
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Transitions")
  void PickUpItem(const FGuid &InstanceId, USceneComponent *AttachTarget, UInteractorComponent *Interactor = nullptr);

  /**
   * 아이템을 내려놓기/던지기 처리합니다.
   *
   * @param InstanceId 내려놓을 아이템의 인스턴스 ID
   * @param Impulse    던지기 시 적용할 임펄스 벡터 (ZeroVector이면 일반 놓기)
   * @param Carrier    아이템을 던지거나 놓은 주체
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|State")
  void DropItem(const FGuid &InstanceId,
                const FVector &Impulse = FVector::ZeroVector,
                UInteractorComponent *Interactor = nullptr);

  // ─── 조회 ───

  /** 월드 내 활성 아이템 목록 반환 */
  const TMap<FGuid, TWeakObjectPtr<AItemBase>> &GetActiveItems() const {
    return ActiveItemsMap;
  }

private:
  /** Tag → Data 레지스트리 */
  UPROPERTY()
  TMap<FGameplayTag, TObjectPtr<UItemData>> ItemDataRegistry;

  /** 월드 내 활성 아이템 레지스트리 맵 */
  TMap<FGuid, TWeakObjectPtr<AItemBase>> ActiveItemsMap;

  /** 기본 아이템 스폰 클래스 (레지스트리에 없을 때 폴백) */
  UPROPERTY()
  TSubclassOf<AItemBase> DefaultItemClass;

  /** 기본 아이템 스폰 클래스 대신 에러 클래스를 사용할지 여부 */
  UPROPERTY()
  bool bUseErrorClassForMissingClass = true;

  /** 에러 아이템 스폰 클래스 (로직에서 클래스 누락 시 스폰) */
  UPROPERTY()
  TSubclassOf<AItemBase> ErrorItemClass;

  /** 스폰 클래스 결정 헬퍼 */
  UClass *ResolveSpawnClass(UItemData *Data,
                            TSubclassOf<AItemBase> ClassOverride) const;

  /** 만료된 WeakPtr 정리 */
  void CleanupStaleEntries();
};
