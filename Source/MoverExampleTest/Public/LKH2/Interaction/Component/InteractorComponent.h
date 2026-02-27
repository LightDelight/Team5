// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "InteractorComponent.generated.h"


class USphereComponent;
class UInteractorPropertyComponent;
class AActor;
class UGridManagerComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractorComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UInteractorComponent();

  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void TryInteract(FGameplayTag IntentTag);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_TryInteract(AActor* Target, FGameplayTag IntentTag);

  /** 액터 소속 상호작용 속성(예: 들고 있는 아이템)을 보관하는 컴포넌트 */
  UPROPERTY()
  TObjectPtr<UInteractorPropertyComponent> PropertyComponent;
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
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
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
  void UpdateTargetInteractable();
  void PollGridTarget();
  void SetSphereTarget(AActor *NewTarget);
  void SetGridTarget(AActor *NewTarget);
  void ProcessInputBuffer(AActor* ServerOverrideTarget = nullptr);

  /** 상호작용 상황(Context)을 생성합니다. */
  FInteractionContext CreateInteractionContext(AActor *Target,
                                   FGameplayTag InteractionTag) const;

  UPROPERTY()
  TArray<AActor *> OverlappingActors;

  // 현재 아웃라인이 켜진(조건에 맞는) 대상들
  UPROPERTY()
  TObjectPtr<AActor> CurrentSphereTarget;

  UPROPERTY()
  TObjectPtr<AActor> CurrentGridTarget;

  UPROPERTY()
  TObjectPtr<UGridManagerComponent> CachedGridManager;

  // Grid Manager에서 액터를 찾을 때 앞쪽으로 얼마나 떨어진 곳을 찾을지 (단위: cm)
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  float GridTargetCheckDistance = 100.0f;

  // 물건을 들었을 때 기본적으로 Sphere 기반 대상 추적을 끕니다. 다만, 지정된 Tag를 가진 아이템을 들었을 때는 추적을 켤지 여부를 결정하는 프로퍼티
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  bool bEnableSphereTraceWhenHoldingItem = true;

  // 물건을 들었을 때 어느 태그를 가져야 SphereTrace를 계속 진행할지
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true", EditCondition = "bEnableSphereTraceWhenHoldingItem"))
  FGameplayTag RequiredSphereTraceTag;

  // 입력 버퍼링 관련 변수들
  FGameplayTag BufferedIntentTag;
  float InputBufferTimer;

  // 매 틱마다 태그 검사를 하는 오버헤드를 줄이기 위한 캐싱 변수
  UPROPERTY()
  TObjectPtr<AActor> CachedCarriedActorForTagCheck;

  bool bCachedShouldSearchSphere = true;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  float InputBufferTimeLimit = 0.2f; // 0.2초 동안 입력을 기억

  // 에디터에서 어느 채널을 사용하여 DetectionSphere에 오버랩 이벤트를 발생시킬지 결정하는 프로퍼티
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Collision",
            meta = (AllowPrivateAccess = "true"))
  TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_PhysicsBody;

  // GridTarget 추적을 위한 타이머 핸들 및 스캔 주기
  FTimerHandle GridTargetTimerHandle;

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Grid",
            meta = (AllowPrivateAccess = "true"))
  float GridTargetPollingInterval = 0.1f;
};
