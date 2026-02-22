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

  UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite,
            Category = "Item|Modules")
  TArray<TObjectPtr<class ULogicModuleBase>> LogicModules;
};
