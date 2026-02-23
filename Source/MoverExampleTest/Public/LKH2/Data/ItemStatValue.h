// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemStatValue.generated.h"

/**
 * ItemStats에 저장되는 값의 타입을 나타내는 열거형
 */
UENUM(BlueprintType)
enum class EItemStatType : uint8 {
  Float UMETA(DisplayName = "Float"),
  Int UMETA(DisplayName = "Int"),
  Bool UMETA(DisplayName = "Bool"),
  Tag UMETA(DisplayName = "GameplayTag"),
  Object UMETA(DisplayName = "Object"),
};

/**
 * GameplayTag를 키(Key)로 사용하는 variant 값 구조체.
 * 하나의 구조체가 여러 타입의 값을 담을 수 있도록
 * EItemStatType으로 현재 활성 타입을 표시합니다.
 */
USTRUCT(BlueprintType)
struct MOVEREXAMPLETEST_API FItemStatValue {
  GENERATED_BODY()

  /** 현재 저장된 값의 타입 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
  EItemStatType Type = EItemStatType::Float;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Stat",
            meta = (EditCondition = "Type == EItemStatType::Float",
                    EditConditionHides))
  float FloatValue = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Stat",
            meta = (EditCondition = "Type == EItemStatType::Int",
                    EditConditionHides))
  int32 IntValue = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Stat",
            meta = (EditCondition = "Type == EItemStatType::Bool",
                    EditConditionHides))
  bool BoolValue = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Stat",
            meta = (EditCondition = "Type == EItemStatType::Tag",
                    EditConditionHides))
  FGameplayTag TagValue;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Stat",
            meta = (EditCondition = "Type == EItemStatType::Object",
                    EditConditionHides))
  TObjectPtr<UObject> ObjectValue = nullptr;

  // --- 편의 생성자 ---
  FItemStatValue() = default;

  static FItemStatValue MakeFloat(float InValue) {
    FItemStatValue V;
    V.Type = EItemStatType::Float;
    V.FloatValue = InValue;
    return V;
  }

  static FItemStatValue MakeInt(int32 InValue) {
    FItemStatValue V;
    V.Type = EItemStatType::Int;
    V.IntValue = InValue;
    return V;
  }

  static FItemStatValue MakeBool(bool InValue) {
    FItemStatValue V;
    V.Type = EItemStatType::Bool;
    V.BoolValue = InValue;
    return V;
  }

  static FItemStatValue MakeTag(FGameplayTag InValue) {
    FItemStatValue V;
    V.Type = EItemStatType::Tag;
    V.TagValue = InValue;
    return V;
  }

  static FItemStatValue MakeObject(UObject *InValue) {
    FItemStatValue V;
    V.Type = EItemStatType::Object;
    V.ObjectValue = InValue;
    return V;
  }
};
