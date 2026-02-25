// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "InteractableComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractableComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UInteractableComponent();

  // 부모 액터(인터페이스)로부터 전달받은 메시지 처리
  bool OnInteract(const FInteractionContext &Context);
  void SetOutlineEnabled(bool bEnabled);

protected:
  // [Pull Pattern] 소유자로부터 직접 조회하므로 자체 보관하지 않습니다.

protected:
  virtual void BeginPlay() override;

private:
  // 경쟁 상태(Race Condition) 관리: 이미 누군가 상호작용 중인지 확인
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<AActor> CurrentInteractor;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  bool bIsInteracting;
};
