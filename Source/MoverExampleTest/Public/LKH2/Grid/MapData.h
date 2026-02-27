// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MapData.generated.h"

class AWorkStationBase;
class UWorkstationData;

/**
 * 베이크된 맵의 워크스테이션 한 개 배치 정보.
 */
USTRUCT(BlueprintType)
struct MOVEREXAMPLETEST_API FMapWorkstationEntry {
  GENERATED_BODY()

  /** 워크스테이션의 그리드 좌표 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map")
  FIntPoint GridCoord;

  /** 스폰할 워크스테이션 블루프린트 클래스 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map")
  TSubclassOf<AWorkStationBase> StationClass;

  /** 워크스테이션에 적용할 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map")
  TObjectPtr<UWorkstationData> StationData;

  /** 워크스테이션의 월드 회전 (베이크 시 캡처) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map")
  FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * 베이크된 맵 데이터.
 * 그리드 설정과 워크스테이션 배치 정보를 보유합니다.
 * 런타임에 GridManagerComponent가 이 에셋을 로드하여 맵을 재구성합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UMapData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  // ── 그리드 설정 ──

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Grid")
  FVector GridOrigin = FVector::ZeroVector;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Grid")
  float CellSize = 200.0f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Grid")
  int32 GridWidth = 10;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Grid")
  int32 GridHeight = 10;

  // ── 워크스테이션 배치 ──

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Workstations")
  TArray<FMapWorkstationEntry> Workstations;
};
