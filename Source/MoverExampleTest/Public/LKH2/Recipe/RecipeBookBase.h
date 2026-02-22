// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RecipeBookBase.generated.h"

class UItemData;

/**
 * UI 시스템에서 레시피를 표기할 때, C++ 캐스팅 없이 범용적으로 화면에 그려줄 수
 * 있도록 정보들을 규격화하는 공용 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FRecipeUIData {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe UI")
  TArray<UItemData *> Inputs;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe UI")
  UItemData *Output = nullptr;

  // 요리 시간, 망치질 횟수 등 커스텀 수치를 태그로 제공
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe UI")
  TMap<FGameplayTag, float> NumericInfo;
};

/**
 * 실제 레시피 데이터 모음집 역할을 하는 추상 클래스입니다. 구조체화된 레시피
 * 데이터들을 배열로 품습니다.
 */
UCLASS(Abstract, BlueprintType)
class MOVEREXAMPLETEST_API URecipeBookBase : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  // UI와의 통신을 위한 가상 함수. 자식 클래스들이 구체적으로 구현하여
  // FRecipeUIData 배열로 반환합니다.
  UFUNCTION(BlueprintCallable, Category = "Recipe")
  virtual TArray<FRecipeUIData> GetAllRecipesForUI() const {
    return TArray<FRecipeUIData>();
  }
};
