// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interaction/Data/ItemRecipeData.h"
#include "ItemRecipeManager.generated.h"

/**
 * 게임 전역의 모든 상호작용 조합(Recipe)을 관리하는 클래스입니다.
 * 게임 인스턴스에 귀속되므로 게임 실행 시 1회 초기화되며 맵 이동에도 유지됩니다.
 * UItemRecipeDataAsset을 통해 데이터 테이블과 정렬 방식을 주입받아 캐싱합니다.
 */
UCLASS()
class MOVEREXAMPLETEST_API UItemRecipeManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 단일 아이템과 상호작용 액션(예: Chop, Cook)을 기반으로, 결과 아이템 태그 목록을 검색합니다. */
	UFUNCTION(BlueprintCallable, Category = "Recipe")
	TArray<FGameplayTag> GetRecipeResultSingle(FGameplayTag InputItemTag, FGameplayTag InteractionTag) const;

	/** 다중 아이템 목록과 상호작용 액션을 기반으로, 결과 아이템 태그 목록을 검색합니다. (순서 무관 매칭 지원) */
	UFUNCTION(BlueprintCallable, Category = "Recipe")
	TArray<FGameplayTag> GetRecipeResultMulti(const TArray<FGameplayTag>& InputItemTags, FGameplayTag InteractionTag) const;

	/** 커스텀 레시피 데이터 에셋을 설정하고 즉시 캐시를 재생성합니다. */
	UFUNCTION(BlueprintCallable, Category = "Recipe")
	void SetRecipeDataAsset(class UItemRecipeDataAsset* NewDataAsset);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 데이터 에셋의 테이블 Row를 캐시에 담고 조건에 맞춰 정렬합니다. */
	void BuildRecipeCache();

private:
	/** 현재 캐싱에 사용된 레시피 데이터 에셋 원본 참조 */
	UPROPERTY(Transient)
	class UItemRecipeDataAsset* CurrentRecipeDataAsset;

	/** 쿼리 성능을 위해 메모리에 정렬된 상태로 들고 있는 레시피 캐시 */
	TArray<FRecipeRow> CachedRecipes;
};
