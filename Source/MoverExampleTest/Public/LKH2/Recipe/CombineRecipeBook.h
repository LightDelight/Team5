// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Recipe/RecipeBookBase.h"
#include "CombineRecipeBook.generated.h"

class UItemData;

/**
 * 조합(Combine) 전용 레시피 데이터 모음 구조체 (에러 방지를 위해 에셋 대신
 * 인라인 데이터 채택)
 */
USTRUCT(BlueprintType)
struct FCombineRecipe {
  GENERATED_BODY()

public:
  // 우선순위가 높을수록 겹치는 재료가 있을 때 먼저 선택됨
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
  int32 Priority = 0;

  // 요구 재료 A
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
  FGameplayTag MaterialA;

  // 요구 재료 B
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
  FGameplayTag MaterialB;

  // 생산 결과 아이템 태그
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
  FGameplayTag ResultItemTag;
};

/**
 * 조합 레시피 묶음 에셋
 * 이 에셋 안에서 FCombineRecipe 배열의 요소를 직접 추가하며 관리합니다.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Combine Recipe Book"))
class MOVEREXAMPLETEST_API UCombineRecipeBook : public URecipeBookBase {
  GENERATED_BODY()

public:
  // 조합 레시피 데이터 목록
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipes")
  TArray<FCombineRecipe> Recipes;

  // UI 출력을 위해 FRecipeUIData 형식으로 변환하여 반환
  virtual TArray<FRecipeUIData> GetAllRecipesForUI() const override;
};
