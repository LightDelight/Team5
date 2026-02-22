// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Logic/LogicModuleBase.h"
#include "RecipeLogicModuleBase.generated.h"

/**
 * 범용 레시피 정렬 및 관리용 템플릿 멤버 함수를 제공하는 기반 로직 모듈
 * (클래스 자체가 제네릭일 필요 없이, 함수만 템플릿으로 제공하여 부모를 가볍게
 * 유지)
 */
UCLASS(Abstract)
class MOVEREXAMPLETEST_API URecipeLogicModuleBase : public ULogicModuleBase {
  GENERATED_BODY()

public:
  URecipeLogicModuleBase();

  // 내림차순 정렬 여부 (예: Priority 값이 클수록 배열의 앞에 오도록 함)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Sort")
  bool bSortDescending = true;

  // LIFO (Last-In-First-Out) 정렬 여부
  // 동등한 Priority를 가질 때, 배열의 뒤쪽에 있던(나중에 추가된) 레시피를
  // 우선시할지 여부
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Sort")
  bool bSortLIFO = true;

protected:
  // --- 에셋 생명주기 (Lifecycle) 관리 ---
  // 1. 게임 빌드 또는 에디터 로드 시 메모리에 올라올 때 단 1회 실행!
  virtual void PostLoad() override;

  // 파생 클래스들이 레시피 데이터를 안전하게 캐싱할 수 있도록 제공하는 함수.
  // PostLoad 및 PostEditChangeProperty에서 자동 호출됩니다.
  virtual void CacheRecipes();

  // 2. 에디터에서 기획자가 레시피 배열이나 숫자를 수정했을 때 즉시 실행!
  virtual void
  PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

protected:
  /**
   * 자식 클래스가 자신의 구조체 배열(TArray<T>)을 만들어 이 함수에
   * 넘기면 설정된 bSortDescending, bSortLIFO 값에 따라 범용적으로 정렬해
   * 줍니다. 에디터 갱신에 안전하도록 포인터가 아닌 값의 배열을 사용합니다.
   *
   * @param InOutRecipes      정렬할 레시피들의 배열
   * @param Pred              우선순위를 비교하는 람다 함수
   */
  template <typename T, typename Predicate>
  void SortRecipes(TArray<T> &InOutRecipes, Predicate Pred) {
    if (bSortLIFO) {
      Algo::Reverse(InOutRecipes);
    }

    bool bDesc = bSortDescending;
    Algo::StableSort(InOutRecipes, [Pred, bDesc](const T &A, const T &B) {
      // 내림차순(Priority 큰 것이 먼저)이면 오름차순용 판단식 Pred(A, B)의 인자
      // 위치를 바꿔 Pred(B, A) (즉, B < A => A > B) 사용. 오름차순이면 원래의
      // Pred(A, B) (즉, A < B) 사용. 주의: !Pred를 사용하면 값이 같을 때 True가
      // 반환되어 Strict Weak Ordering 원칙이 깨집니다.
      return bDesc ? Pred(B, A) : Pred(A, B);
    });
  }
};
