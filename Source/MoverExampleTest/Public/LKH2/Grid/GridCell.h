// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridCell.generated.h"

class AActor;

/**
 * 그리드 한 칸을 나타내는 구조체.
 * 그리드 좌표, 월드 중심점, 해당 셀에 등록된 액터 참조를 보유합니다.
 */
USTRUCT(BlueprintType)
struct MOVEREXAMPLETEST_API FGridCell {
  GENERATED_BODY()

  FGridCell() : GridCoord(FIntPoint::ZeroValue), WorldCenter(FVector::ZeroVector) {}

  FGridCell(const FIntPoint &InCoord, const FVector &InCenter)
      : GridCoord(InCoord), WorldCenter(InCenter) {}

  /** 이 셀의 그리드 좌표 (X, Y) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
  FIntPoint GridCoord;

  /** 이 셀의 월드 공간 중심점 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
  FVector WorldCenter;

  /** 이 셀에 등록된 액터 (워크스테이션 등). 없으면 null */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
  TWeakObjectPtr<AActor> OccupyingActor;

  /** 셀이 비어있는지 확인 */
  bool IsEmpty() const {
    return !OccupyingActor.IsValid();
  }
};
