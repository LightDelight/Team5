// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class UItemData : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  FText ItemName;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  TObjectPtr<UStaticMesh> ItemMesh;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
  float ItemWeight = 10.0f;

  /** Sphere 콜리전 반지름 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Collision")
  float SphereRadius = 32.0f;

  /** 루트(SphereCollision) 기준 메쉬의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Collision")
  FVector MeshRelativeLocation = FVector::ZeroVector;

  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Item|Modules")
  TArray<TObjectPtr<class ULogicModuleBase>> LogicModules;
};
