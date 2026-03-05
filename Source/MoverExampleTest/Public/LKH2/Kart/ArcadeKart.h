// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ArcadeKart.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * [최종 완성형 v15] 소프트코딩 최적화 버전
 * 모든 물리 및 보간 파라미터가 에디터에서 수정 가능하도록 설계되었습니다.
 * To do : 타 클라이언트 카트의 미래 예측. 현재는 과거를 따라갈 뿐이라 필연적인 어색함 발생.
 */
UCLASS()
class AArcadeKart : public APawn
{
    GENERATED_BODY()

public:
    AArcadeKart();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
    virtual void BeginPlay() override;

public:
    // --- 컴포넌트 ---

    // 실제 물리 연산 및 충돌의 주체가 되는 루트 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* BoxCollision;

    // 시각적 표현 메쉬 (물리 박스와 분리되어 부드럽게 보간됨)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* WheelPosition_FL; 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* WheelPosition_FR; 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* WheelPosition_RL; 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* WheelPosition_RR; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComp;

    // --- 디버그 설정 ---

    // 서버 위치를 붉은 박스로 표시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Debug")
    bool bShowServerPosition = true;

    // 스냅 타겟 위치를 자색 구체로 표시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Debug")
    bool bShowSnapDebug = false;

    // 예측 광선을 노란색으로 표시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Debug")
    bool bShowPredictionDebug = false;

    // --- 시각적 보간 및 스무딩 (Soft-coded) ---

    // 직선 주행 시 원격 카트 추격 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float SmoothingSpeed_Straight = 7.0f; 

    // 회전 중 원격 카트 추격 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float SmoothingSpeed_Turn = 25.0f;

    // 스냅 해제 후 메쉬가 루트로 돌아오는 기본 보간 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float RestoreLerpSpeed = 15.0f;

    // 스냅 중 메쉬가 목표 지점을 쫓아가는 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float SnapChaseSpeed = 15.0f;
    
    // 스냅 중 메쉬가 목표 회전을 쫓아가는 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float RotationSmoothingSpeed = 15.0f;
    
    // 레이턴시에 따른 시각적 추격 가중치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float LagBoostMultiplier = 0.3f;

    // 서버와 거리가 이 이상 벌어지면 순간이동
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|Interpolation")
    float TeleportDistanceThreshold = 500.0f;
    
    // 고스트 반동(Ghost Bounce)이 감쇄되어 정지하는 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float GhostBounceDamping = 2.0f;
    
    // 일정 수치 이하 고스트 반동(Ghost Bounce)은 빠르게 감쇄
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float GhostBounceThresholdSq = 100.0f;
    
    // GhostBounceThresholdSq 이하 고스트 반동을 감쇄할 속도 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap", meta = (ClampMin = "1.0", UIMin = "1.0", UMAx = "10.0"))
    float LowVelocityDampingMultiplier = 2.0f;
    
    // 충격량에 따른 가상 반동 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float GhostBounceMultiplier = 1.6f;

    // 스냅 가중치(Alpha)가 원래대로 돌아오는 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float VisualRestoreSpeed = 2.5f;

    // 스냅 보정을 시작할 최소 거리 차이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float HitSnapDistanceThreshold = 150.0f;

    // 스냅 시 강제로 확보할 최소 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float HitSnapOffsetDistance = 90.0f;
    
    // 충돌 반응 속도 (빠를수록 즉각적, 느릴수록 부드러움)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Visuals|GhostSnap")
    float OffsetInterpSpeed = 5.0f;
    
    // Throttle 입력 스무딩 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Input")
    float ThrottleSmoothingSpeed = 2.0f;

    // Steering 입력 스무딩 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Input")
    float SteeringSmoothingSpeed = 10.0f;
    
    // --- 예측 충돌 설정 ---

    // 미리 충돌을 감지할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction")
    bool bUsePredictiveCollision = true; 

    // 핑(Ping) 기반 동적 예측 시간 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction")
    bool bUseDynamicPrediction = true;

    // 예측 충돌 시 카트 크기 배율 (1.15 = 15% 더 크게 감지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction")
    float PredictionSweepScale = 1.15f;

    // 고정 예측 시간 (동적 예측 미사용 시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction", meta = (EditCondition = "!bUseDynamicPrediction"))
    float FixedPredictionTime = 0.15f;

    // 동적 예측 최소값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction", meta = (EditCondition = "bUseDynamicPrediction"))
    float MinPredictionTime = 0.05f;

    // 동적 예측 최대값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction", meta = (EditCondition = "bUseDynamicPrediction"))
    float MaxPredictionTime = 0.35f;

    // 예측 계산 기본 오프셋
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction", meta = (EditCondition = "bUseDynamicPrediction"))
    float PredictionTimeOffset = 0.05f;

    // 핑(ms) 당 가중치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Prediction", meta = (EditCondition = "bUseDynamicPrediction"))
    float PingMultiplier = 0.001f;

    // 실제 적용 중인 예측 시간 (디버그용)
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Kart Physics|Prediction")
    float PredictionTime;

    // --- 네트워크 최적화 설정 ---

    // 위치 정보를 전송할 최소 거리 변화량 (제곱값)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Network")
    float NetSignificantMoveDistSq = 2.0f;

    // 회전 정보를 전송할 최소 각도 변화량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Network")
    float NetSignificantRotDeg = 0.5f;

    // 전송 빈도 제한 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Network")
    float MinNetUpdateInterval = 0.033f;

    // 충돌 패킷 유효 거리 검증값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Network")
    float MaxNetworkPositionErrorSq = 1000000.0f;

    // --- 물리 튜닝 파라미터 ---

    // 타겟 카트의 예상 질량 (물리가 꺼진 프록시 계산용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float AssumedRemoteKartMass = 80.0f;

    // 환경 기물(물리 물체) 충돌 시 힘 전달 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float EnvironmentImpactMultiplier = 0.7f;

    // 최소 유효 충돌 속도 제곱
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float MinImpactSpeedSq = 1200.0f;

    // 충돌 쿨다운
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float CollisionCooldown = 0.25f;

    // 상대방 밀기 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float PushImpulseMultiplier = 350.0f;

    // 상대방 띄우기 힘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float PushUpwardForce = 200.0f;

    // 자신 반동 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Tuning|Collision")
    float SelfBounceMultiplier = 180.0f;

    // 서스펜션 길이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Suspension")
    float TraceLength = 45.0f;

    // 서스펜션 강성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Suspension")
    float SuspensionStiffness = 35000.0f;

    // 서스펜션 댐핑
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Suspension")
    float SuspensionDamping = 60.0f;

    // 가속 힘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Engine")
    float AccelerationForce = 90000.0f;

    // 조향 토크
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Steering")
    float TurningTorque = 65000.0f;

    // 공중 조향 배율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Steering")
    float AirSteeringMultiplier = 0.15f;

    // 주행 그립
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Grip")
    float TireGripFactor = 4.5f;

    // 드리프트 그립
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Grip")
    float DriftGripFactor = 1.2f;

    // 마찰 작용점 상하 오프셋
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Grip")
    float FrictionHeightOffset = 30.0f; 

    // 추가 중력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Gravity")
    float ExtraGravity = 120000.0f;
    
    // 무게 중심
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart Physics|Gravity")
    FVector CenterOfMass = FVector(0.0f, 0.0f, 0.0f);

    // --- 입력 및 네트워크 RPC ---

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SteerAction; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DriftAction; 

    UFUNCTION(BlueprintCallable, Category = "Kart Mechanics")
    bool IsDrifting() const { return bIsDrifting; }

    UFUNCTION(BlueprintCallable, Category = "Kart Mechanics")
    void SetThrottleInput(float NewThrottle);

    UFUNCTION(Server, Unreliable, WithValidation)
    void ServerUpdateTransform(FVector NewLoc, FRotator NewRot);

    UPROPERTY(Replicated)
    FVector_NetQuantize100 ReplicatedLocation;

    UPROPERTY(Replicated)
    FRotator ReplicatedRotation;

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetThrottle(float Value);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetSteering(float Value);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetDrift(bool bNewDrift);

    UFUNCTION(Server, Unreliable, WithValidation)
    void ServerNotifyHit(AArcadeKart* OtherKart, FVector ImpactImpulse, AArcadeKart* Attacker);

    UFUNCTION(Server, Unreliable, WithValidation)
    void ServerHitPhysicsObject(UPrimitiveComponent* HitComp, FVector ImpactImpulse, FVector HitLocation);

    UFUNCTION(Client, Reliable)
    void ClientReceiveHit(FVector ImpactImpulse, AArcadeKart* Attacker);

private:
    UPROPERTY(Replicated)
    float ThrottleInput;

    UPROPERTY(Replicated)
    float SteerInput;

    UPROPERTY(Replicated)
    bool bIsDrifting;

    float CurrentThrottle;
    float CurrentSteer;
    bool bIsGrounded; 
    bool bWasMoving;
    
    TArray<USceneComponent*> WheelComponents;
    FVector MeshRelativeOffset;
    float LastCollisionTime;

    FVector LastSentLocation;
    FRotator LastSentRotation;
    float LastNetUpdateTime;

    float VisualSnapWeight; 
    FVector SnapAnchorOffset;
    TWeakObjectPtr<AActor> SnapAnchorActor;
    FVector GhostBounceVelocity;
    FVector CurrentVisualSnapLocation;

    // 최종적으로 도달해야 할 목표 오프셋 (RPC에서 즉시 갱신됨)
    FVector TargetSnapAnchorOffset;

    // 현재 프레임에 적용 중인 오프셋 (Tick에서 Target을 향해 보간됨)
    FVector CurrentSnapAnchorOffset;
    
    void ApplySuspensionForces(float DeltaTime);
    void ApplyMovementForces(float DeltaTime);
    void CheckPredictiveCollision(float DeltaTime);

    void OnMove(const struct FInputActionValue& Value);
    void OnSteer(const struct FInputActionValue& Value);
    void OnDriftStart(const struct FInputActionValue& Value);
    void OnDriftEnd(const struct FInputActionValue& Value);
};