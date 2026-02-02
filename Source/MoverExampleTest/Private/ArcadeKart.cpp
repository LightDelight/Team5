// Fill out your copyright notice in the Description page of Project Settings.

#include "ArcadeKart.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "ProfilingDebugging/CsvProfiler.h"

CSV_DEFINE_CATEGORY(KartSnap, true);

AArcadeKart::AArcadeKart()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // [중요] 엔진 기본 움직임 복제 비활성화
    SetReplicateMovement(false);

    SetNetUpdateFrequency(60.0f);
    SetMinNetUpdateFrequency(30.0f);

    // 1. BoxCollision
    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    RootComponent = BoxCollision;

    BoxCollision->SetBoxExtent(FVector(40.f, 30.f, 30.f));
    BoxCollision->SetSimulatePhysics(true);
    BoxCollision->SetMassOverrideInKg(NAME_None, 80.0f, true);
    BoxCollision->SetLinearDamping(0.5f);
    BoxCollision->SetAngularDamping(3.0f);
    BoxCollision->BodyInstance.bUseCCD = true;
    BoxCollision->SetCollisionProfileName(TEXT("PhysicsActor"));
    BoxCollision->SetIsReplicated(false);
    BoxCollision->SetNotifyRigidBodyCollision(true);
    BoxCollision->SetGenerateOverlapEvents(true);

    // 2. BodyMesh
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(BoxCollision);
    BodyMesh->SetSimulatePhysics(false);
    BodyMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // 3. Wheels
    float FWheelX = 30.0f;
    float FWheelY = 25.0f;
    float FWheelZ = -20.0f;

    float RWheelX = -30.0f;
    float RWheelY = 25.0f;
    float RWheelZ = -20.0f;

    WheelPosition_FL = CreateDefaultSubobject<USceneComponent>(TEXT("WheelPosition_FL"));
    WheelPosition_FL->SetupAttachment(BoxCollision);
    WheelPosition_FL->SetRelativeLocation(FVector(FWheelX, -FWheelY, FWheelZ));

    WheelPosition_FR = CreateDefaultSubobject<USceneComponent>(TEXT("WheelPosition_FR"));
    WheelPosition_FR->SetupAttachment(BoxCollision);
    WheelPosition_FR->SetRelativeLocation(FVector(FWheelX, FWheelY, FWheelZ));

    WheelPosition_RL = CreateDefaultSubobject<USceneComponent>(TEXT("WheelPosition_RL"));
    WheelPosition_RL->SetupAttachment(BoxCollision);
    WheelPosition_RL->SetRelativeLocation(FVector(RWheelX, -RWheelY, RWheelZ));

    WheelPosition_RR = CreateDefaultSubobject<USceneComponent>(TEXT("WheelPosition_RR"));
    WheelPosition_RR->SetupAttachment(BoxCollision);
    WheelPosition_RR->SetRelativeLocation(FVector(RWheelX, RWheelY, RWheelZ));

    // 4. Camera
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(BodyMesh);
    SpringArmComp->TargetArmLength = 400.0f;
    SpringArmComp->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    SpringArmComp->bInheritPitch = false;
    SpringArmComp->bInheritRoll = false;
    SpringArmComp->bEnableCameraLag = true;
    SpringArmComp->CameraLagSpeed = 10.0f;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp);

    // 변수 초기화
    ThrottleInput = 0.0f;
    SteerInput = 0.0f;
    bIsDrifting = false;
    bIsGrounded = false;
    bWasMoving = false;
    CurrentThrottle = 0.0f;
    CurrentSteer = 0.0f;
    LastCollisionTime = 0.0f;

    VisualSnapWeight = 0.0f;
    VisualRestoreSpeed = 2.0f;
    GhostBounceVelocity = FVector::ZeroVector;

    // [쇼핑카트 물리 튜닝]
    TraceLength = 40.0f;
    SuspensionStiffness = 30000.0f;
    SuspensionDamping = 50.0f;
    AccelerationForce = 80000.0f;
    TurningTorque = 60000.0f;
    TireGripFactor = 4.0f;
    DriftGripFactor = 1.0f;
    FrictionHeightOffset = 30.0f;
    ExtraGravity = 100000.0f;

    // [보간 설정]
    LagBoostMultiplier = 0.5f;
    TeleportDistanceThreshold = 500.0f;
    bShowServerPosition = true;

    // [수정] 기본적으로 켜둠 (BP에서 끄더라도 코드상 true로 시작하여 원격 프록시에서도 보이게 함)
    bShowSnapDebug = true;

    // [예측 충돌]
    bUsePredictiveCollision = true;
    bUseDynamicPrediction = true;
    FixedPredictionTime = 0.15f;
    MinPredictionTime = 0.05f;
    MaxPredictionTime = 0.3f;
    PredictionTimeOffset = 0.05f;
    PingMultiplier = 0.001f;
    PredictionTime = 0.15f;

    // [수정] 기본적으로 켜둠
    bShowPredictionDebug = true;

    // [입력 스무딩]
    ThrottleSmoothingSpeed = 2.0f;
    SteeringSmoothingSpeed = 10.0f;

    // [충돌 및 네트워크 상수]
    MinImpactSpeedSq = 1000.0f;
    CollisionCooldown = 0.2f;
    PushImpulseMultiplier = 300.0f;
    PushUpwardForce = 200.0f;
    SelfBounceMultiplier = 150.0f;
    MaxNetworkPositionErrorSq = 1000000.0f;

    HitSnapDistanceThreshold = 200.0f;
    HitSnapOffsetDistance = 80.0f;

    SmoothingSpeed_Straight = 5.0f;
    SmoothingSpeed_Turn = 20.0f;
}

void AArcadeKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

#if UE_BUILD_SHIPPING
    DOREPLIFETIME_CONDITION(AArcadeKart, ReplicatedLocation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(AArcadeKart, ReplicatedRotation, COND_SkipOwner);
#else
    DOREPLIFETIME(AArcadeKart, ReplicatedLocation);
    DOREPLIFETIME(AArcadeKart, ReplicatedRotation);
#endif

    DOREPLIFETIME(AArcadeKart, bIsDrifting);
    DOREPLIFETIME_CONDITION(AArcadeKart, ThrottleInput, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(AArcadeKart, SteerInput, COND_SkipOwner);
}

void AArcadeKart::BeginPlay()
{
    Super::BeginPlay();
    WheelComponents = { WheelPosition_FL, WheelPosition_FR, WheelPosition_RL, WheelPosition_RR };

    BoxCollision->SetCenterOfMass(CenterOfMass);

    // 시각적 보간 준비
    MeshRelativeOffset = BodyMesh->GetRelativeLocation();

    // [중요] 메쉬 분리를 위해 Absolute 설정
    BodyMesh->SetUsingAbsoluteLocation(true);
    BodyMesh->SetUsingAbsoluteRotation(true);

    FVector TargetLoc = BoxCollision->GetComponentTransform().TransformPosition(MeshRelativeOffset);
    FRotator TargetRot = BoxCollision->GetComponentRotation();
    BodyMesh->SetWorldLocationAndRotation(TargetLoc, TargetRot);

    LastSentLocation = GetActorLocation();
    LastSentRotation = GetActorRotation();

    if (IsLocallyControlled())
    {
        BoxCollision->SetSimulatePhysics(true);
        BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        BoxCollision->SetNotifyRigidBodyCollision(true);
    }
    else
    {
        BoxCollision->SetSimulatePhysics(false);
        BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        BoxCollision->SetNotifyRigidBodyCollision(true);
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void AArcadeKart::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 입력값 스무딩
    CurrentThrottle = FMath::FInterpTo(CurrentThrottle, ThrottleInput, DeltaTime, ThrottleSmoothingSpeed);
    CurrentSteer = FMath::FInterpTo(CurrentSteer, SteerInput, DeltaTime, SteeringSmoothingSpeed);

    if (IsLocallyControlled())
    {
        if (bUseDynamicPrediction)
        {
            float CurrentPing = 0.0f;
            if (APlayerState* PS = GetPlayerState()) CurrentPing = PS->GetPingInMilliseconds();
            PredictionTime = FMath::Clamp((CurrentPing * PingMultiplier) + PredictionTimeOffset, MinPredictionTime, MaxPredictionTime);
        }
        else
        {
            PredictionTime = FixedPredictionTime;
        }

        // 로컬 플레이어 로직 (기존 유지)
        ApplySuspensionForces(DeltaTime);
        ApplyMovementForces(DeltaTime);
        CheckPredictiveCollision(DeltaTime);

        // 3. 네트워크 전송량 제한 로직 (소프트코딩 임계값 사용)
        float Now = GetWorld()->GetTimeSeconds();
        if (Now - LastNetUpdateTime >= MinNetUpdateInterval)
        {
            FVector CurrentLoc = GetActorLocation();
            FRotator CurrentRot = GetActorRotation();

            // 움직임이 유의미한지 확인
            bool bSignificantMove = FVector::DistSquared(CurrentLoc, LastSentLocation) > NetSignificantMoveDistSq;
            bool bSignificantRot = !CurrentRot.Equals(LastSentRotation, NetSignificantRotDeg);

            // [보완] 속도가 있는지 확인 (멈추는 순간을 잡기 위함)
            bool bIsMoving = !GetVelocity().IsNearlyZero();

            if (bSignificantMove || bSignificantRot || (bWasMoving && !bIsMoving))
            {
                ServerUpdateTransform(CurrentLoc, CurrentRot);

                LastSentLocation = CurrentLoc;
                LastSentRotation = CurrentRot;
                LastNetUpdateTime = Now;
                bWasMoving = bIsMoving; // 이전 프레임의 이동 상태 저장
            }
        }

        FVector TargetLoc = BoxCollision->GetComponentTransform().TransformPosition(MeshRelativeOffset);
        FRotator TargetRot = BoxCollision->GetComponentRotation();
        BodyMesh->SetWorldLocationAndRotation(TargetLoc, TargetRot);
    }
    else
    {
        // [원격 플레이어 개선 로직]

        // 1. Root Component 업데이트 (기존 유지)
        {
            float DistanceSq = FVector::DistSquared(GetActorLocation(), ReplicatedLocation);

            // 임계값 이상이면 즉시 순간이동
            if (DistanceSq > FMath::Square(TeleportDistanceThreshold))
            {
                SetActorLocationAndRotation(ReplicatedLocation, ReplicatedRotation);
            }
            else
            {
                float BaseSpeed = (FMath::Abs(SteerInput) > 0.1f) ? SmoothingSpeed_Turn : SmoothingSpeed_Straight;
                float DynamicSpeed = BaseSpeed + (FMath::Sqrt(DistanceSq) * LagBoostMultiplier);

                FVector NewLoc = FMath::VInterpTo(GetActorLocation(), ReplicatedLocation, DeltaTime, DynamicSpeed);
                FRotator NewRot = FMath::RInterpTo(GetActorRotation(), ReplicatedRotation, DeltaTime, DynamicSpeed);
                SetActorLocationAndRotation(NewLoc, NewRot);
            }
        }

        // 2. Body Mesh 시각적 보간 (Snap & Ghost Physics)
        if (VisualSnapWeight > 0.001f)
        {
            // [Ghost Physics] 가상 반동 속도 업데이트
            if (!GhostBounceVelocity.IsZero())
            {
                // 1. 기본 감쇄 속도 결정
                float TargetDamping = GhostBounceDamping;

                // 2. 속도가 낮으면 배율을 적용해 더 빨리 0으로 수렴하게 만듦 (Soft Cutoff)
                if (GhostBounceVelocity.SizeSquared() < GhostBounceThresholdSq)
                {
                    TargetDamping *= LowVelocityDampingMultiplier;
                }

                // 3. 실제 보간 적용
                GhostBounceVelocity = FMath::VInterpTo(GhostBounceVelocity, FVector::ZeroVector, DeltaTime, TargetDamping);
                CurrentVisualSnapLocation += GhostBounceVelocity * DeltaTime;

                // 4. (선택사항) 정말 미세한 값은 물리 엔진 오차 방지를 위해 0으로 컷
                if (GhostBounceVelocity.IsNearlyZero(0.1f)) GhostBounceVelocity = FVector::ZeroVector;
            }

            // [Soft Snap Entry] 목표 오프셋으로 아주 빠르게 이동 (순간이동 방지 핵심)
            // SnapAnchorOffset을 즉시 바꾸지 않고, Tick에서 쫓아가게 함으로써 '빠른 슬라이드' 연출
            // 이 로직을 위해 SnapAnchorOffset은 변수로서 계속 보간됩니다.
            /* 주의: SnapAnchorOffset은 .h 파일에 정의된 변수입니다.
               TargetSnapAnchorOffset이라는 임시 변수를 .h에 추가하여 사용하거나
               간단하게 현재 SnapAnchorOffset을 목표값으로 부드럽게 밀어줍니다.
            */

            // Step 1: '현재 오프셋'을 '목표 오프셋'으로 부드럽게 이동 (Interp Speed 조절로 반응성 튜닝)
            // 이미 스냅 중인데 새로운 충돌이 와서 Target이 바뀌어도, Current는 부드럽게 선회함.
            CurrentSnapAnchorOffset = FMath::VInterpTo(CurrentSnapAnchorOffset, TargetSnapAnchorOffset, DeltaTime, OffsetInterpSpeed);

            // Step 2: 스냅 종료 시 가중치 감소
            // (충돌 후 일정 시간이 지나거나, Target과 가까워지면 Weight를 줄이는 로직 필요)
            VisualSnapWeight = FMath::FInterpTo(VisualSnapWeight, 0.0f, DeltaTime, VisualRestoreSpeed);

            // Step 3: 최종 월드 좌표 계산
            FVector FinalSnapTarget;
            if (SnapAnchorActor.IsValid())
            {
                // Victim 기준 상대 좌표 -> 월드 좌표 변환
                // 이때 TargetOffset이 아닌 부드러운 CurrentOffset을 사용
                FVector AnchorWorldPos = SnapAnchorActor->GetActorTransform().TransformPosition(CurrentSnapAnchorOffset);
                FinalSnapTarget = AnchorWorldPos + CurrentVisualSnapLocation;
            }
            else
            {
                // Victim이 사라지면 현재 위치 유지 (Pop 방지)
                FinalSnapTarget = BodyMesh->GetComponentLocation();
                VisualSnapWeight = 0.0f; // 강제 복귀 시작
            }

            // Step 4: 메쉬 이동 (기존 로직 활용하되, 입력값만 위에서 구한 부드러운 값 사용)
            FVector RootTargetLoc = BoxCollision->GetComponentTransform().TransformPosition(MeshRelativeOffset);

            // Alpha: 스냅 강도
            float Alpha = FMath::Clamp(VisualSnapWeight * VisualSnapWeight, 0.0f, 1.0f);

            // Root 위치와 스냅 위치 사이를 Alpha로 블렌딩
            FVector DesiredBlendedLoc = FMath::Lerp(RootTargetLoc, FinalSnapTarget, Alpha);

            // 최종적으로 메쉬에 적용 (VInterpTo를 한 번 더 거쳐서 아주 미세한 떨림까지 잡음)
            FVector SmoothBlendedLoc = FMath::VInterpTo(BodyMesh->GetComponentLocation(), DesiredBlendedLoc, DeltaTime, SnapChaseSpeed);
            FRotator BlendedRot = FMath::RInterpTo(BodyMesh->GetComponentRotation(), ReplicatedRotation, DeltaTime, RotationSmoothingSpeed);

            BodyMesh->SetWorldLocationAndRotation(SmoothBlendedLoc, BlendedRot);

#if !UE_BUILD_SHIPPING
            if (bShowSnapDebug)
            {
                DrawDebugSphere(GetWorld(), FinalSnapTarget, 30.0f, 8, FColor::Magenta, false, -1.0f, 0, 1.0f);
                DrawDebugLine(GetWorld(), RootTargetLoc, FinalSnapTarget, FColor::Cyan, false, -1.0f, 0, 2.0f);
            }
#endif
        }
        else
        {
            // 스냅 종료 후 복귀 (기존 유지)
            FVector RootTargetLoc = BoxCollision->GetComponentTransform().TransformPosition(MeshRelativeOffset);
            FRotator RootTargetRot = BoxCollision->GetComponentRotation();

            FVector SmoothLoc = FMath::VInterpTo(BodyMesh->GetComponentLocation(), RootTargetLoc, DeltaTime, RestoreLerpSpeed);
            FRotator SmoothRot = FMath::RInterpTo(BodyMesh->GetComponentRotation(), RootTargetRot, DeltaTime, RestoreLerpSpeed);

            BodyMesh->SetWorldLocationAndRotation(SmoothLoc, SmoothRot);

            // 로직 리셋
            CurrentVisualSnapLocation = FVector::ZeroVector;
            GhostBounceVelocity = FVector::ZeroVector;
        }
    }

#if !UE_BUILD_SHIPPING
    if (bShowServerPosition && BoxCollision)
    {
        DrawDebugBox(GetWorld(), ReplicatedLocation, BoxCollision->GetScaledBoxExtent(), ReplicatedRotation.Quaternion(), FColor::Red, false, -1.0f, 0, 1.0f);
    }
#endif

    // 예측 충돌 스냅 후 피해자 클라이언트에서 가해자 카트 위치 변화 체크용 기록
#if !UE_BUILD_SHIPPING
    if (!IsLocallyControlled())
    {
        if (VisualSnapWeight > 0.001f)
        {
            // 1. 스냅 가중치 (0~1 사이에서 부드럽게 변하는지 확인)
            CSV_CUSTOM_STAT(KartSnap, Weight, VisualSnapWeight, ECsvCustomStatOp::Set);

            // 2. 목표 지점과 현재 스무딩 지점 사이의 거리 (이 값이 급격히 튀었다가 서서히 0이 되어야 정상)
            float SmoothLagDistance = FVector::Dist(TargetSnapAnchorOffset, CurrentSnapAnchorOffset);
            CSV_CUSTOM_STAT(KartSnap, SmoothLag, SmoothLagDistance, ECsvCustomStatOp::Set);

            // 3. 실제 메쉬가 렌더링되는 위치와 '이상적인 목표 위치'와의 오차
            // (이 값이 작을수록 유저 눈에는 정확해 보임)
            if (SnapAnchorActor.IsValid())
            {
                FVector IdealWorldPos = SnapAnchorActor->GetActorTransform().TransformPosition(TargetSnapAnchorOffset);
                float VisualError = FVector::Dist(BodyMesh->GetComponentLocation(), IdealWorldPos);
                CSV_CUSTOM_STAT(KartSnap, RealVisualError, VisualError, ECsvCustomStatOp::Set);
            }

            // 4. Ghost Physics 속도 (반동이 잘 줄어드는지)
            CSV_CUSTOM_STAT(KartSnap, GhostSpeed, GhostBounceVelocity.Size(), ECsvCustomStatOp::Set);
        }
        else
        {
            // 스냅 중이 아닐 때는 0으로 기록 (그래프 끊김 방지)
            //CSV_CUSTOM_STAT(KartSnap, Weight, 0.0f, ECsvCustomStatOp::Set);
            //CSV_CUSTOM_STAT(KartSnap, SmoothLag, 0.0f, ECsvCustomStatOp::Set);
        }
    }
#endif
}

void AArcadeKart::CheckPredictiveCollision(float DeltaTime)
{
    // 1. 기능 스위치가 꺼져있으면 즉시 리턴
    if (!bUsePredictiveCollision) return;

    if (GetWorld()->GetTimeSeconds() - LastCollisionTime < CollisionCooldown) return;

    FVector Velocity = BoxCollision->GetPhysicsLinearVelocity();
    float SpeedSq = Velocity.SizeSquared();

    if (SpeedSq < MinImpactSpeedSq) return;

    FVector Start = BoxCollision->GetComponentLocation();
    FVector End = Start + (Velocity * PredictionTime);

    // 충돌 박스 크기 (기존 대비 1.1배)
    FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxCollision->GetScaledBoxExtent() * 1.1f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // Sweep 실행
    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        BoxCollision->GetComponentQuat(),
        ECC_Pawn, // Pawn 채널 사용 (카트끼리 감지)
        BoxShape,
        Params
    );

    // [중요] 바닥 및 정적 장애물 필터링
    bool bValidTarget = false;
    if (bHit && HitResult.GetActor())
    {
        // 타겟이 카트거나, 물리 시뮬레이션 중인 동적 물체인 경우만 '유효한 충돌'로 인정
        if (HitResult.GetActor()->IsA(AArcadeKart::StaticClass()) ||
            (HitResult.Component.IsValid() && HitResult.Component->IsSimulatingPhysics()))
        {
            bValidTarget = true;
        }
    }

#if !UE_BUILD_SHIPPING
    if (bShowPredictionDebug)
    {
        // 바닥에 부딪힌 것은 빨간색(무시), 유효한 타겟(카트 등)은 초록색으로 표시
        DrawDebugBox(GetWorld(), End, BoxShape.GetExtent(), BoxCollision->GetComponentQuat(), bValidTarget ? FColor::Green : FColor::Red, false, -1.0f, 0, 1.0f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, -1.0f, 0, 2.0f);
    }
#endif

    // 유효한 타겟일 때만 NotifyHit 호출
    if (bValidTarget)
    {
        NotifyHit(BoxCollision, HitResult.GetActor(), HitResult.GetComponent(), false, HitResult.Location, HitResult.ImpactNormal, FVector::ZeroVector, HitResult);
    }
}

void AArcadeKart::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCollisionTime < CollisionCooldown) return;

    if (IsLocallyControlled() && Other && OtherComp)
    {
        FVector MyVelocity = BoxCollision->GetPhysicsLinearVelocity();
        float SpeedSq = MyVelocity.SizeSquared();

        if (SpeedSq > MinImpactSpeedSq)
        {
            LastCollisionTime = CurrentTime;

            FVector ImpactDir = MyVelocity.GetSafeNormal();
            float ImpactPower = FMath::Clamp(FMath::Sqrt(SpeedSq) * 0.05f, 0.5f, 3.0f);

            FVector PushImpulse = ImpactDir * BoxCollision->GetMass() * PushImpulseMultiplier * ImpactPower;
            PushImpulse.Z = BoxCollision->GetMass() * PushUpwardForce;

            FVector BounceImpulse = HitNormal * BoxCollision->GetMass() * SelfBounceMultiplier;
            BounceImpulse.Z = BoxCollision->GetMass() * 50.0f;

            if (Other->IsA(AArcadeKart::StaticClass()))
            {
                AArcadeKart* OtherKart = Cast<AArcadeKart>(Other);
                ServerNotifyHit(OtherKart, PushImpulse, this);
                BoxCollision->AddImpulse(BounceImpulse);
            }
            else if (OtherComp->IsSimulatingPhysics())
            {
                ServerHitPhysicsObject(OtherComp, PushImpulse * 0.8f, HitLocation);
                BoxCollision->AddImpulse(BounceImpulse * 0.5f);
            }
            else
            {
                BoxCollision->AddImpulse(BounceImpulse);
            }
        }
    }
}

bool AArcadeKart::ServerNotifyHit_Validate(AArcadeKart* OtherKart, FVector ImpactImpulse, AArcadeKart* Attacker)
{
    if (!OtherKart || !Attacker) return false;
    return true;
}

void AArcadeKart::ServerNotifyHit_Implementation(AArcadeKart* OtherKart, FVector ImpactImpulse, AArcadeKart* Attacker)
{
    if (OtherKart)
    {
        float DistanceSq = FVector::DistSquared(GetActorLocation(), OtherKart->GetActorLocation());
        if (DistanceSq < MaxNetworkPositionErrorSq)
        {
            OtherKart->ClientReceiveHit(ImpactImpulse, Attacker);
        }
    }
}

void AArcadeKart::ClientReceiveHit_Implementation(FVector ImpactImpulse, AArcadeKart* Attacker)
{
    if (BoxCollision && BoxCollision->IsSimulatingPhysics())
    {
        BoxCollision->AddImpulse(ImpactImpulse);
    }

    if (Attacker && Attacker != this)
    {
        FVector MyLoc = GetActorLocation();
        FVector AttackerLoc = Attacker->GetActorLocation();
        float DistSq = FVector::DistSquared(MyLoc, AttackerLoc);
        float SnapThresholdSq = HitSnapDistanceThreshold * HitSnapDistanceThreshold;

        if (DistSq > SnapThresholdSq)
        {
            FVector HitDir = -ImpactImpulse.GetSafeNormal();
            float MinSafeSeparation = 120.0f;
            if (BoxCollision && Attacker->BoxCollision)
            {
                MinSafeSeparation = (BoxCollision->GetScaledBoxExtent().Size() + Attacker->BoxCollision->GetScaledBoxExtent().Size()) * 0.8f;
            }

            float FinalSnapDistance = FMath::Max(HitSnapOffsetDistance, MinSafeSeparation);
            FVector CorrectPos = MyLoc + (HitDir * FinalSnapDistance);
            CorrectPos.Z = FMath::Lerp(AttackerLoc.Z, MyLoc.Z, 0.5f);

            // [핵심 수정] 
            // 1. 상대방 좌표계 기준의 목표 오프셋 계산
            FVector NewTargetOffset = this->GetActorTransform().InverseTransformPosition(CorrectPos);

            // 2. 만약 처음 스냅(첫 충돌)이라면, 시작점을 현재 메쉬 위치로 맞춰서 'Pop' 방지
            if (Attacker->VisualSnapWeight <= 0.01f)
            {
                FVector CurrentAttackerMeshLoc = Attacker->BodyMesh->GetComponentLocation();
                Attacker->CurrentSnapAnchorOffset = this->GetActorTransform().InverseTransformPosition(CurrentAttackerMeshLoc);
            }

            // 3. 최종 목표값 갱신 (이미 스냅 중이어도 이것만 바꾸면 됨)
            Attacker->TargetSnapAnchorOffset = NewTargetOffset;
            Attacker->SnapAnchorActor = this;

            // 4. 가중치는 1.0을 향해 가도록 설정 (Tick에서 처리)
            // (주의: 여기서 강제로 1.0으로 세팅하지 말고, Tick에서 자연스럽게 증가시키는 게 더 부드러움)
            Attacker->VisualSnapWeight = 1.0f;

            float AttackerMass = 80.0f;
            // 다른 카트의 Physics Simulate는 꺼져 있으므로 질량을 받아오지 못함.
            // if (Attacker->BoxCollision) AttackerMass = Attacker->BoxCollision->GetMass();
            Attacker->GhostBounceVelocity = (ImpactImpulse / AttackerMass) * 1.5f;

            // [Soft Snap] 앵커 설정
            Attacker->SnapAnchorActor = this;

            /* [핵심 수정]
               SnapAnchorOffset을 즉시 CorrectPos로 설정하는 것이 아니라,
               현재 메쉬가 있는 '상대적 위치'에서 시작하게 하여 순간이동을 방지합니다.
            */
            FVector CurrentAttackerMeshLoc = Attacker->BodyMesh->GetComponentLocation();
            Attacker->SnapAnchorOffset = this->GetActorTransform().InverseTransformPosition(CurrentAttackerMeshLoc);

            // 그리고 Tick에서 SnapAnchorOffset을 목표 오프셋으로 빠르게 보정하거나, 
            // 아래처럼 목표 오프셋 자체를 'Target' 개념으로 사용할 수 있습니다.
            // 여기서는 Tick 로직의 단순화를 위해 SnapAnchorOffset 자체를 CorrectPos로 두되,
            // Tick에서 BodyMesh의 위치를 VInterpTo 하는 방식으로 순간이동을 해결합니다.
            
            Attacker->CurrentVisualSnapLocation = FVector::ZeroVector;

            // 가중치를 1.0으로 설정 (이제 Tick의 VInterpTo가 자연스럽게 목표 지점으로 밀어줌)
            Attacker->VisualSnapWeight = 1.0f;

#if !UE_BUILD_SHIPPING
            if (bShowSnapDebug)
            {
                DrawDebugSphere(GetWorld(), CorrectPos, 50.0f, 12, FColor::Magenta, false, 2.0f);
            }
#endif
        }
        else if (Attacker->VisualSnapWeight > 0.1f)
        {
            // 이미 스냅 중인데 가까이서 또 부딪힌 경우, 속도만 중첩시켜 튕겨냄
            float AttackerMass = 80.0f;
            // 다른 카트의 Physics Simulate는 꺼져 있으므로 질량을 받아오지 못함.
            // if (Attacker->BoxCollision) AttackerMass = Attacker->BoxCollision->GetMass();
            Attacker->GhostBounceVelocity += (ImpactImpulse / AttackerMass);
            Attacker->VisualSnapWeight = FMath::Max(Attacker->VisualSnapWeight, 0.5f);
        }
    }
}

// ... (나머지 RPC 동일) ...
bool AArcadeKart::ServerHitPhysicsObject_Validate(UPrimitiveComponent* HitComp, FVector ImpactImpulse, FVector HitLocation) { return true; }
void AArcadeKart::ServerHitPhysicsObject_Implementation(UPrimitiveComponent* HitComp, FVector ImpactImpulse, FVector HitLocation)
{
    if (HitComp && HitComp->IsSimulatingPhysics()) HitComp->AddImpulseAtLocation(ImpactImpulse, HitLocation);
}

bool AArcadeKart::ServerUpdateTransform_Validate(FVector NewLoc, FRotator NewRot) { return true; }
void AArcadeKart::ServerUpdateTransform_Implementation(FVector NewLoc, FRotator NewRot)
{
    ReplicatedLocation = NewLoc;
    ReplicatedRotation = NewRot;
}

void AArcadeKart::ServerSetThrottle_Implementation(float Value) { ThrottleInput = Value; }
void AArcadeKart::ServerSetSteering_Implementation(float Value) { SteerInput = Value; }
void AArcadeKart::ServerSetDrift_Implementation(bool bNewDrift) { bIsDrifting = bNewDrift; }

bool AArcadeKart::ServerSetDrift_Validate(bool bNewDrift) { return true; }
bool AArcadeKart::ServerSetThrottle_Validate(float Value) { return true; }
bool AArcadeKart::ServerSetSteering_Validate(float Value) { return true; }

void AArcadeKart::ApplySuspensionForces(float DeltaTime)
{
    bIsGrounded = false;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    float CurrentGrip = bIsDrifting ? DriftGripFactor : TireGripFactor;
    float MassPerWheel = BoxCollision->GetMass() / 4.0f;

    for (USceneComponent* WheelComp : WheelComponents)
    {
        if (!WheelComp) continue;

        FVector RayStart = WheelComp->GetComponentLocation();
        FVector RayEnd = RayStart - (BoxCollision->GetUpVector() * TraceLength);

        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic, QueryParams);

        if (bHit)
        {
            bIsGrounded = true;

            FVector VelocityAtWheel = BoxCollision->GetPhysicsLinearVelocityAtPoint(RayStart);
            float UpVelocity = FVector::DotProduct(VelocityAtWheel, BoxCollision->GetUpVector());

            float Distance = (RayStart - Hit.Location).Length();
            float CompressionRatio = 1.0f - (Distance / TraceLength);

            float ForceMagnitude = (CompressionRatio * SuspensionStiffness) - (UpVelocity * SuspensionDamping);
            ForceMagnitude = FMath::Max(0.0f, ForceMagnitude);

            FVector SuspensionForce = BoxCollision->GetUpVector() * ForceMagnitude;
            BoxCollision->AddForceAtLocation(SuspensionForce, RayStart);

            FVector RightVector = BoxCollision->GetRightVector();
            float LateralSpeed = FVector::DotProduct(VelocityAtWheel, RightVector);

            FVector FrictionForce = -RightVector * LateralSpeed * MassPerWheel * CurrentGrip;
            FVector FrictionLoc = RayStart + (BoxCollision->GetUpVector() * FrictionHeightOffset);

            BoxCollision->AddForceAtLocation(FrictionForce, FrictionLoc);
        }
    }
}

void AArcadeKart::ApplyMovementForces(float DeltaTime)
{
    if (bIsGrounded)
    {
        if (FMath::Abs(CurrentThrottle) > 0.01f)
        {
            FVector ForwardForce = BoxCollision->GetForwardVector() * CurrentThrottle * AccelerationForce;
            BoxCollision->AddForce(ForwardForce);
        }

        if (FMath::Abs(CurrentSteer) > 0.01f)
        {
            float DirectionMult = (CurrentThrottle < -0.1f) ? -1.0f : 1.0f;
            FVector Torque = FVector(0.f, 0.f, 1.f) * CurrentSteer * DirectionMult * TurningTorque;
            BoxCollision->AddTorqueInRadians(Torque, NAME_None, false);
        }
    }
    else
    {
        BoxCollision->AddForce(FVector(0, 0, -ExtraGravity));
        if (FMath::Abs(CurrentSteer) > 0.01f)
        {
            FVector AirTorque = FVector(0.f, 0.f, 1.f) * CurrentSteer * TurningTorque * 0.1f;
            BoxCollision->AddTorqueInRadians(AirTorque, NAME_None, false);
        }
    }
}

void AArcadeKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArcadeKart::OnMove);
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AArcadeKart::OnMove);
        EnhancedInput->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AArcadeKart::OnSteer);
        EnhancedInput->BindAction(SteerAction, ETriggerEvent::Completed, this, &AArcadeKart::OnSteer);
        EnhancedInput->BindAction(DriftAction, ETriggerEvent::Started, this, &AArcadeKart::OnDriftStart);
        EnhancedInput->BindAction(DriftAction, ETriggerEvent::Completed, this, &AArcadeKart::OnDriftEnd);
    }
}

void AArcadeKart::OnMove(const FInputActionValue& Value)
{
    ThrottleInput = Value.Get<float>();
    ServerSetThrottle(Value.Get<float>());
}

void AArcadeKart::OnSteer(const FInputActionValue& Value)
{
    SteerInput = Value.Get<float>();
    ServerSetSteering(Value.Get<float>());
}

void AArcadeKart::OnDriftStart(const FInputActionValue& Value)
{
    bIsDrifting = true;
    ServerSetDrift(true);
}

void AArcadeKart::OnDriftEnd(const FInputActionValue& Value)
{
    bIsDrifting = false;
    ServerSetDrift(false);
}

void AArcadeKart::SetThrottleInput(float NewThrottle)
{
    ThrottleInput = NewThrottle;
    ServerSetThrottle(NewThrottle);
}