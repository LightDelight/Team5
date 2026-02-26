// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "InteractionContextInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionContextInterface : public UInterface
{
  GENERATED_BODY()
};

/**
 * 상호작용에 필요한 모든 컨텍스트 데이터를 담는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FInteractionContext
{
  GENERATED_BODY()

  /** 상호작용 시도자 (예: 플레이어) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<AActor> Interactor;

  /** 상호작용 유형 태그 (Interaction.Interact, Interaction.Throw 등) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FGameplayTag InteractionTag;

  /** 상호작용 의도를 발생시킨 컴포넌트 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<UActorComponent> InteractorComp;

  /** 시도자의 프로퍼티(상태) 컴포넌트 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<UActorComponent> InteractorPropertyComp;

  /** 추가적인 대상 액터 (예: 아이템을 올려둘 워크스테이션 등, 선택 사항) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<AActor> TargetActor;

  /** 대상의 상호작용 컴포넌트 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<UActorComponent> InteractableComp;

  /** 대상의 프로퍼티 컴포넌트 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<UActorComponent> InteractablePropertyComp;

  /** 대상의 로직 컨텍스트(블랙보드) 컴포넌트 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  TObjectPtr<UActorComponent> ContextComp;

  FInteractionContext() {}
  FInteractionContext(AActor *InInteractor, FGameplayTag InTag)
      : Interactor(InInteractor), InteractionTag(InTag) {}
};

/**
 * 상호작용 가능한 객체가 구현해야 할 인터페이스
 */
class IInteractionContextInterface
{
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
