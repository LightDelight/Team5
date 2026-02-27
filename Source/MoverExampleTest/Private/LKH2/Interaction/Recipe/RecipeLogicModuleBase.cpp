// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Recipe/RecipeLogicModuleBase.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"

URecipeLogicModuleBase::URecipeLogicModuleBase() {
  bSortDescending = true;
  bSortLIFO = true;
}

void URecipeLogicModuleBase::InitializeLogic(AActor *Context, const class ULogicEntityDataBase* EntityData) {
  Super::InitializeLogic(Context, EntityData);

  // 인스턴스화 될 때(BeginPlay 이후) InteractableComponent를 찾고 DataAsset을 통해 다시 한 번 캐싱 시도
  if (Context) {
    if (UInteractableComponent* InteractableComp = Context->FindComponentByClass<UInteractableComponent>()) {
      // DataAsset을 받아 파생 클래스의 로직에서 필요한 레시피 데이터를 찾도록 넘김
      CacheRecipes(EntityData);
    }
  }
}

void URecipeLogicModuleBase::CacheRecipes(const class ULogicEntityDataBase* EntityData) {
  // 자식 모듈들이 이 함수를 오버라이드하여 RecipeBook들을 순회하고 단일 레시피
  // 복사 캐싱을 조립한 뒤 부모의 SortRecipes 템플릿 함수를 호출해야 합니다.
}