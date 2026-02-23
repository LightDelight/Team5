// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorkstationData.generated.h"

/**
 * 워크스테이션(작업대) 데이터를 정의하는 클래스
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UWorkstationData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation")
  FText WorkstationName;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation")
  TObjectPtr<UStaticMesh> WorkstationMesh;

  /** Box 콜리전 크기 (Half-Extent) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Collision")
  FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);

  /** 루트(RootMesh) 기준 BoxCollision의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Collision")
  FVector BoxRelativeLocation = FVector::ZeroVector;

  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Workstation|Modules")
  TArray<TObjectPtr<class ULogicModuleBase>> LogicModules;
};
