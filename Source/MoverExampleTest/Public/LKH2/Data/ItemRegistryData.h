// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemRegistryData.generated.h"

class UItemData;
class AItemBase;

/**
 * 아이템 레지스트리 항목.
 * GameplayTag를 키로 하여 ItemData와 스폰 클래스를 매핑합니다.
 */
USTRUCT(BlueprintType)
struct FItemRegistryEntry {
  GENERATED_BODY()

  /** 아이템 식별용 GameplayTag */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Registry")
  FGameplayTag ItemTag;

  /** 아이템 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Registry")
  TObjectPtr<UItemData> ItemData;

  /** 스폰할 액터 클래스 (미지정 시 DefaultItemClass 사용) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Registry")
  TSubclassOf<AItemBase> ItemClass;
};

/**
 * 아이템 레지스트리 데이터 에셋.
 * ItemManagerSubsystem 초기화 시 로드되어
 * Tag ↔ Data ↔ Class 매핑을 일괄 등록합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UItemRegistryData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** 아이템 매핑 항목 배열 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  TArray<FItemRegistryEntry> Entries;

  /** 레지스트리에 클래스가 지정되지 않은 아이템의 기본 스폰 클래스 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  TSubclassOf<AItemBase> DefaultItemClass;
};
