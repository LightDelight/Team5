// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionContextInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionContextInterface : public UInterface {
  GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EInteractionType : uint8 {
  Interact UMETA(DisplayName = "Interact (Pick Up or Drop)"),
  Throw UMETA(DisplayName = "Throw")
};

/**
 * 상호작용에 필요한 모든 컨텍스트 데이터를 담는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FInteractionContext {
  GENERATED_BODY()

  /** 상호작용 시도자 (예: 플레이어) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<AActor> Interactor;

  /** 상호작용 유형 (Grab, Drop, Throw 등) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  EInteractionType InteractionType = EInteractionType::Interact;

  /** 현재 손에 무언가를 들고 있는지 여부 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bIsHandOccupied = false;

  /** 현재 손에 들고 있는 아이템 (없으면 nullptr) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<AActor> InHandActor;

  /** 던지기 등 물리 동작 시 적용할 속도/방향 (선택 사항) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FVector Velocity = FVector::ZeroVector;

  /** 추가적인 대상 액터 (예: 아이템을 올려둘 워크스테이션 등, 선택 사항) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<AActor> TargetActor;

  FInteractionContext() {}
  FInteractionContext(AActor *InInteractor, EInteractionType InType)
      : Interactor(InInteractor), InteractionType(InType) {}
};

/**
 * 상호작용 가능한 객체가 구현해야 할 인터페이스
 */
class IInteractionContextInterface {
  GENERATED_BODY()

public:
  // 누군가가 이 액터에 대해 상호작용을 시도했을 때 호출됩니다. 
  // 컨텍스트 정보를 확인하여 적절한 동작(로직 모듈로의 위임)을 수행합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  bool OnInteract(const FInteractionContext &Context);

  // 아웃라인(외곽선) 표시 여부를 설정합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void SetOutlineEnabled(bool bEnabled);
};
