// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PresetData.generated.h"

class ULogicModuleBase;

/**
 * 프리셋 대상 타입
 */
UENUM(BlueprintType)
enum class EPresetDataType : uint8 {
  Item UMETA(DisplayName = "Item"),
  Workstation UMETA(DisplayName = "Workstation"),
};

/**
 * 데이터 에셋(ItemData/WorkstationData)의 기본 모듈 구성을 정의하는 프리셋.
 * 어떤 종류의 데이터인지(Item/Workstation)와
 * 기본적으로 포함할 로직 모듈 클래스 목록을 설정합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UPresetData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  /** 이 프리셋의 대상 타입 (Item 또는 Workstation) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
  EPresetDataType PresetType = EPresetDataType::Item;

  /** 기본적으로 포함할 로직 모듈 클래스 목록 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset|Modules")
  TArray<TSubclassOf<ULogicModuleBase>> DefaultModuleClasses;
};
