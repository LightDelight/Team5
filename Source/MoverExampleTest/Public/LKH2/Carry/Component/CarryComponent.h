// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "CarryComponent.generated.h"


class USphereComponent;
class AActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCarryComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UCarryComponent();

  // 줍기/놓기 버퍼링 입력
  UFUNCTION(BlueprintCallable, Category = "Carry")
  void TryPickupOrDrop();

  // 던지기 버퍼링 입력
  UFUNCTION(BlueprintCallable, Category = "Carry")
  void TryThrow();

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_TryPickupOrDrop(AActor *Target);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_TryThrow(AActor *Target);

  // 상호작용 등을 위한 아이템 강제 제어 함수
  UFUNCTION(BlueprintCallable, Category = "Carry")
  AActor *GetCarriedActor() const;

  UFUNCTION(BlueprintCallable, Category = "Carry")
  void ForceDrop();

  UFUNCTION(BlueprintCallable, Category = "Carry")
  void ForceEquip(AActor *ItemToEquip);

protected:
  virtual void BeginPlay() override;

protected:
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // 평소에는 꺼져있다가 오버랩 감지 시에만 켜져서 세부 추적 시작
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  // 감지 범위. 평소에는 충돌 채널 이벤트만 받음
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carry")
  TObjectPtr<USphereComponent> DetectionSphere;

  UFUNCTION()
  void OnDetectionBeginOverlap(UPrimitiveComponent *OverlappedComponent,
                               AActor *OtherActor,
                               UPrimitiveComponent *OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep,
                               const FHitResult &SweepResult);

  UFUNCTION()
  void OnDetectionEndOverlap(UPrimitiveComponent *OverlappedComponent,
                             AActor *OtherActor, UPrimitiveComponent *OtherComp,
                             int32 OtherBodyIndex);

private:
  void UpdateTargetCarryable();
  void SetTarget(AActor *NewTarget);
  void ProcessInputBuffer(AActor *Target);

  /** 상호작용 상황(Context)을 생성합니다. */
  FCarryContext CreateCarryContext(AActor *Target,
                                   ECarryInteractionType Type) const;

  UPROPERTY()
  TArray<AActor *> OverlappingActors;

  // 현재 바라보고 있는 (아웃라인이 켜진) 대상
  UPROPERTY()
  TObjectPtr<AActor> CurrentTarget;

  // 현재 들고 있는 대상
  UPROPERTY(ReplicatedUsing = OnRep_CarriedActor)
  TObjectPtr<AActor> CarriedActor;

  UFUNCTION()
  void OnRep_CarriedActor(AActor *OldCarriedActor);

  // 입력 버퍼링 관련 변수들
  bool bWantsPickupOrDrop;
  bool bWantsThrow;
  float InputBufferTimer;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Carry",
            meta = (AllowPrivateAccess = "true"))
  float InputBufferTimeLimit = 0.2f; // 0.2초 동안 입력을 기억
};
