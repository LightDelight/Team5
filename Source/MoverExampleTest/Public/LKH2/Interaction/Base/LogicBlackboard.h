// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LogicBlackboard.generated.h"


class UObject;

/**
 * 블랙보드에 저장되는 단일 오브젝트 데이터 항목 (Fast Array Serializer Item)
 */
USTRUCT(BlueprintType)
struct FLogicBlackboardObjectEntry : public FFastArraySerializerItem {
  GENERATED_BODY()

  FLogicBlackboardObjectEntry()
      : Key(FGameplayTag::EmptyTag), ObjectValue(nullptr) {}

  FLogicBlackboardObjectEntry(const FGameplayTag &InKey, UObject *InObject)
      : Key(InKey), ObjectValue(InObject) {}

  /** 이 데이터를 식별하는 키 (예: Carry.Workstation.StoredItem) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  FGameplayTag Key;

  /** 실제 보관 중인 오브젝트 데이터 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  TObjectPtr<UObject> ObjectValue;
};

/**
 * 멀티플레이어 환경에서 엔진 차원의 차등 동기화(Delta Serialization)를
 * 안전하고 퍼포먼스 좋게 진행하기 위한 범용 오브젝트 블랙보드 배열 구조체.
 */
USTRUCT(BlueprintType)
struct FLogicBlackboardObjectSerializer : public FFastArraySerializer {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  TArray<FLogicBlackboardObjectEntry> Items;

  /** 네트워크 직렬화 델타 함수 */
  bool NetDeltaSerialize(FNetDeltaSerializeInfo &DeltaParms) {
    return FFastArraySerializer::FastArrayDeltaSerialize<
        FLogicBlackboardObjectEntry, FLogicBlackboardObjectSerializer>(
        Items, DeltaParms, *this);
  }

  /**
   * 주어진 GameplayTag 키에 해당하는 오브젝트 포인터를 갱신하거나 새로
   * 추가합니다.
   * @return 데이터가 추가 또는 갱신되어 네트워크 마킹(MarkItemDirty)이 불렸는지
   * 여부
   */
  bool SetObject(const FGameplayTag &Key, UObject *Value) {
    if (!Key.IsValid()) {
      return false;
    }

    // 1. 이미 같은 키가 있는지 탐색
    for (FLogicBlackboardObjectEntry &Entry : Items) {
      if (Entry.Key == Key) {
        if (Entry.ObjectValue != Value) {
          Entry.ObjectValue = Value;
          MarkItemDirty(Entry); // 중요: 이 라인이 있어야 클라이언트로 복제됨
          return true;
        }
        return false; // 이미 같은 값
      }
    }

    // 2. 새로운 키인 경우 추가
    FLogicBlackboardObjectEntry NewEntry(Key, Value);
    Items.Add(NewEntry);
    MarkArrayDirty(); // 중요: 배열 구조 자체의 변경 알림
    return true;
  }

  /** 주어진 GameplayTag 키에 해당하는 오브젝트 포인터를 찾아서 반환합니다. */
  UObject *GetObject(const FGameplayTag &Key) const {
    for (const FLogicBlackboardObjectEntry &Entry : Items) {
      if (Entry.Key == Key) {
        return Entry.ObjectValue;
      }
    }
    return nullptr;
  }
};

/**
 * 블랙보드에 저장되는 단일 스탯 데이터 항목
 */
USTRUCT(BlueprintType)
struct FLogicBlackboardStatEntry : public FFastArraySerializerItem {
  GENERATED_BODY()

  FLogicBlackboardStatEntry() : Key(FGameplayTag::EmptyTag) {}

  FLogicBlackboardStatEntry(const FGameplayTag &InKey,
                            const FItemStatValue &InValue)
      : Key(InKey), Value(InValue) {}

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  FGameplayTag Key;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  FItemStatValue Value;
};

/**
 * 런타임 가변 수치(Stats)를 동기화하기 위한 시리얼라이저
 */
USTRUCT(BlueprintType)
struct FLogicBlackboardStatSerializer : public FFastArraySerializer {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
  TArray<FLogicBlackboardStatEntry> Items;

  bool NetDeltaSerialize(FNetDeltaSerializeInfo &DeltaParms) {
    return FFastArraySerializer::FastArrayDeltaSerialize<
        FLogicBlackboardStatEntry, FLogicBlackboardStatSerializer>(
        Items, DeltaParms, *this);
  }

  bool SetStat(const FGameplayTag &Key, const FItemStatValue &Value) {
    if (!Key.IsValid())
      return false;

    for (FLogicBlackboardStatEntry &Entry : Items) {
      if (Entry.Key == Key) {
        // 간단한 비교 로직 (필요시 FItemStatValue에 operator== 추가 고려)
        // 여기서는 구조체 복사 후 변경 마킹 위주로 처리
        Entry.Value = Value;
        MarkItemDirty(Entry);
        return true;
      }
    }

    FLogicBlackboardStatEntry NewEntry(Key, Value);
    Items.Add(NewEntry);
    MarkArrayDirty();
    return true;
  }

  const FItemStatValue *GetStat(const FGameplayTag &Key) const {
    for (const FLogicBlackboardStatEntry &Entry : Items) {
      if (Entry.Key == Key) {
        return &Entry.Value;
      }
    }
    return nullptr;
  }
};

/**
 * 메커니즘 등록
 */
template <>
struct TStructOpsTypeTraits<FLogicBlackboardStatSerializer>
    : public TStructOpsTypeTraitsBase2<FLogicBlackboardStatSerializer> {
  enum { WithNetDeltaSerializer = true };
};

/**
 * 위에서 정의한 FastArraySerializer 메커니즘이 언리얼 엔진의 RPC 리플리케이션
 * 시스템에 제대로 연결되도록 델타 직렬화 특성(Trait)을 선언합니다.
 */
template <>
struct TStructOpsTypeTraits<FLogicBlackboardObjectSerializer>
    : public TStructOpsTypeTraitsBase2<FLogicBlackboardObjectSerializer> {
  enum {
    WithNetDeltaSerializer = true,
  };
};

/**
 * 최종 활용하게 될 여러 데이터 타입 지원 종합 블랙보드입니다.
 * 컴포넌트 내부 등에 선언하여 사용합니다.
 */
USTRUCT(BlueprintType)
struct MOVEREXAMPLETEST_API FLogicBlackboard {
  GENERATED_BODY()

public:
  /** 오브젝트 타입 데이터를 담는 동기화 지원 블랙보드 */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blackboard")
  FLogicBlackboardObjectSerializer ObjectBlackboard;

  /** 런타임 가변 수치(Stats)를 담는 동기화 지원 블랙보드 */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blackboard")
  FLogicBlackboardStatSerializer Stats;

  /**
   * 필요할 경우 이곳에 Float, Int 기반의 Serializer 도 동일하게 만들어서 넣을
   * 수 있습니다. 예: FLogicBlackboardFloatSerializer FloatBlackboard;
   */
};
