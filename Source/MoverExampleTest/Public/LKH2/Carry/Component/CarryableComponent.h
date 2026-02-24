// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "CarryableComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCarryableComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UCarryableComponent();

  // 부모 액터(인터페이스)로부터 전달받은 메시지 처리
  bool OnCarryInteract(const FCarryContext &Context);
  void SetOutlineEnabled(bool bEnabled);

protected:
  // [Pull Pattern] 소유자로부터 직접 조회하므로 자체 보관하지 않습니다.

protected:
  virtual void BeginPlay() override;

private:
  // 경쟁 상태(Race Condition) 관리: 이미 누군가 들고 있는지 확인
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carry",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<AActor> CurrentCarrier;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carry",
            meta = (AllowPrivateAccess = "true"))
  bool bIsCarried;
};
