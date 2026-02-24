// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CarryInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCarryInterface : public UInterface {
  GENERATED_BODY()
};

UENUM(BlueprintType)
enum class ECarryInteractionType : uint8 {
  Interact UMETA(DisplayName = "Interact (Pick Up or Drop)"),
  Throw UMETA(DisplayName = "Throw")
};

/**
 * 캐리 상호작용에 필요한 모든 컨텍스트 데이터를 담는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FCarryContext {
  GENERATED_BODY()

  /** 상호작용 시도자 (예: 플레이어) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  TObjectPtr<AActor> Interactor;

  /** 상호작용 유형 (Grab, Drop, Throw 등) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  ECarryInteractionType InteractionType = ECarryInteractionType::Interact;

  /** 현재 손에 무언가를 들고 있는지 여부 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  bool bIsHandOccupied = false;

  /** 현재 손에 들고 있는 아이템 (없으면 nullptr) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  TObjectPtr<AActor> InHandActor;

  /** 던지기 등 물리 동작 시 적용할 속도/방향 (선택 사항) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  FVector Velocity = FVector::ZeroVector;

  /** 추가적인 대상 액터 (예: 아이템을 올려둘 워크스테이션 등, 선택 사항) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carry")
  TObjectPtr<AActor> TargetActor;

  FCarryContext() {}
  FCarryContext(AActor *InInteractor, ECarryInteractionType InType)
      : Interactor(InInteractor), InteractionType(InType) {}
};

/**
 * 캐리(들기) 가능한 객체가 구현해야 할 인터페이스
 */
class ICarryInterface {
  GENERATED_BODY()

public:
  // 누군가가 이 액터에 대해 들기/놓기/투척 등의 캐리 상호작용을 시도했을 때
  // 호출됩니다. 컨텍스트 정보를 확인하여 적절한 동작(로직 모듈로의 위임)을
  // 수행합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  bool OnCarryInteract(const FCarryContext &Context);

  // 아웃라인(외곽선) 표시 여부를 설정합니다.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Carry")
  void SetOutlineEnabled(bool bEnabled);
};
