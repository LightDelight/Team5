// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemRecipeData.generated.h"

/**
 * 아이템 조합/상호작용 결과를 정의하는 데이터 테이블 행 구조체입니다.
 */
USTRUCT(BlueprintType)
struct MOVEREXAMPLETEST_API FRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 상호작용의 대상이 되는 입력 아이템 태그 목록 (예: [Item.Dough, Item.Tomato]) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FGameplayTag> InputItemTags;

	/** 입력 아이템에 가해지는 상호작용 종류 태그 (예: Context.Bake) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FGameplayTag InteractionTag;

	/** 상호작용 결과로 생성되어야 할 아이템 태그 목록 (예: [Item.Pizza, Item.Trash]) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FGameplayTag> OutputItemTags;

	/** 레시피 조합 우선순위 (높을수록 먼저 탐색) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 Priority = 0;

	FRecipeRow() {}
};

/**
 * 데이터 테이블(UDataTable)을 감싸고, 해당 테이블 데이터를 읽을 때 적용할
 * 정렬 규칙(Priority 우선순위 등)을 함께 정의하는 데이터 에셋입니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UItemRecipeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 실제 레시피 데이터가 들어있는 데이터 테이블 (FRecipeRow 구조체 기반) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	class UDataTable* RecipeTable;

	/** 내림차순 정렬 여부 (Priority 값이 클수록 먼저 확인) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Sort")
	bool bSortDescending = true;

	/** LIFO (Last-In-First-Out) 정렬 여부 (동일 우선순위일 때 나중에 추가된 레시피를 우선시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Sort")
	bool bSortLIFO = true;
};
