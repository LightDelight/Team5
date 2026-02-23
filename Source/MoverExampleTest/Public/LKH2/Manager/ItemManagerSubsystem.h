// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "ItemManagerSubsystem.generated.h"

class UItemData;
class AItemBase;
class UItemRegistryData;

/**
 * 아이템 생성·파괴·추적을 중앙 관리하는 월드 서브시스템.
 *
 * 주요 책임:
 *  - GameplayTag ↔ UItemData ↔ TSubclassOf<AItemBase> 레지스트리
 *  - SpawnActorDeferred → SetItemDataAndApply → FinishSpawning 표준 스폰 파이프라인
 *  - 월드 내 활성 아이템 추적
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
  void RegisterItemData(FGameplayTag ItemTag, UItemData *Data,
                        TSubclassOf<AItemBase> ItemClass = nullptr);

  /** Tag로 UItemData 조회 (미등록 시 nullptr) */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ItemManager|Registry")
  UItemData *FindItemData(FGameplayTag ItemTag) const;

  // ─── 스폰 API ───

  /**
   * GameplayTag로 아이템을 스폰합니다.
   * 레지스트리에서 Data/Class를 자동 조회합니다.
   *
   * @return 스폰된 AItemBase* (실패 시 nullptr)
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Spawn")
  AItemBase *SpawnItem(FGameplayTag ItemTag, const FTransform &Transform);

  /**
   * UItemData를 직접 지정하여 아이템을 스폰합니다.
   *
   * @param ClassOverride  스폰 클래스 직접 지정 (nullptr이면 레지스트리/기본값 사용)
   * @return 스폰된 AItemBase* (실패 시 nullptr)
   */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Spawn")
  AItemBase *SpawnItemFromData(UItemData *Data, const FTransform &Transform,
                               TSubclassOf<AItemBase> ClassOverride = nullptr);

  // ─── 파괴 ───

  /** 아이템을 파괴하고 추적 목록에서 제거합니다. */
  UFUNCTION(BlueprintCallable, Category = "ItemManager|Lifecycle")
  void DestroyItem(AItemBase *Item);

  // ─── 조회 ───

  /** 월드 내 활성 아이템 목록 반환 */
  const TArray<TWeakObjectPtr<AItemBase>> &GetActiveItems() const {
    return ActiveItems;
  }

private:
  /** Tag → Data 레지스트리 */
  UPROPERTY()
  TMap<FGameplayTag, TObjectPtr<UItemData>> ItemDataRegistry;

  /** Data → 스폰 클래스 레지스트리 */
  UPROPERTY()
  TMap<TObjectPtr<UItemData>, TSubclassOf<AItemBase>> ItemClassRegistry;

  /** 월드 내 활성 아이템 추적 (WeakPtr로 수동 파괴 감지) */
  TArray<TWeakObjectPtr<AItemBase>> ActiveItems;

  /** 기본 아이템 스폰 클래스 (레지스트리에 없을 때 폴백) */
  UPROPERTY()
  TSubclassOf<AItemBase> DefaultItemClass;

  /** 스폰 클래스 결정 헬퍼 */
  UClass *ResolveSpawnClass(UItemData *Data,
                            TSubclassOf<AItemBase> ClassOverride) const;

  /** 만료된 WeakPtr 정리 */
  void CleanupStaleEntries();
};
