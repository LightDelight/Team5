// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemRegistryData.generated.h"

class UItemData;
class AItemBase;

/**
 * 아이템 레지스트리 데이터 에셋.
 * ItemManagerSubsystem 초기화 시 로드되어
 * 각 ItemData의 Tag를 읽어 자동 등록합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UItemRegistryData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** 등록할 아이템 데이터 목록 (각 데이터의 ItemTag를 키로 자동 등록됨) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  TArray<TObjectPtr<UItemData>> ItemDatas;

  /** 레지스트리에 클래스가 지정되지 않은 아이템의 기본 스폰 클래스 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  TSubclassOf<AItemBase> DefaultItemClass;

  /** 클래스가 지정되지 않았을 때 ErrorItemClass를 스폰할지 여부 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  bool bUseErrorClassForMissingClass = true;

  /** 에러 상황 (클래스 누락, 레시피 매칭 실패 등) 시 스폰할 클래스 (시각적 디버깅용) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
  TSubclassOf<AItemBase> ErrorItemClass;
};
