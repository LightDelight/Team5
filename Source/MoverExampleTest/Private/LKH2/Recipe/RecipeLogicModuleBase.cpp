// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Recipe/RecipeLogicModuleBase.h"

URecipeLogicModuleBase::URecipeLogicModuleBase() {
  bSortDescending = true;
  bSortLIFO = true;
}

void URecipeLogicModuleBase::PostLoad() {
  Super::PostLoad();
  InitializeLogic();
}
void URecipeLogicModuleBase::InitializeLogic() {
  // 자식 모듈들이 이 함수를 오버라이드하여 RecipeBook들을 순회하고 단일 레시피
  // 복사 캐싱을 조립한 뒤 부모의 SortRecipes 템플릿 함수를 호출해야 합니다.
}

// 2. 에디터에서 기획자가 레시피 배열이나 숫자를 수정했을 때 즉시 실행!
void URecipeLogicModuleBase::PostEditChangeProperty(
    FPropertyChangedEvent &PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
#if WITH_EDITOR
  InitializeLogic(); // 수정한 즉시 다시 캐싱 및 정렬됨
#endif
}