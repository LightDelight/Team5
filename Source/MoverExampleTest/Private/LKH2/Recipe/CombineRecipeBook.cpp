// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Recipe/CombineRecipeBook.h"

TArray<FRecipeUIData> UCombineRecipeBook::GetAllRecipesForUI() const {
  TArray<FRecipeUIData> UIList;

  for (const FCombineRecipe &Recipe : Recipes) {
    FRecipeUIData NewUIData;

    if (Recipe.MaterialA) {
      NewUIData.Inputs.Add(Recipe.MaterialA);
    }
    if (Recipe.MaterialB) {
      NewUIData.Inputs.Add(Recipe.MaterialB);
    }

    NewUIData.Output = Recipe.ResultItemData;

    UIList.Add(NewUIData);
  }

  return UIList;
}
