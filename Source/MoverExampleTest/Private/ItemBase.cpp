#include "ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

AItemBase::AItemBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true); // 언리얼 기본 움직임 동기화 사용

    // 루트: 충돌체 (서버 논리 위치)
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->SetSimulatePhysics(true);
    CapsuleComp->SetCollisionProfileName(TEXT("PhysicsActor"));
    // Z축, 회전 잠금 (Physics Constraints)
    CapsuleComp->BodyInstance.bLockZTranslation = true;
    CapsuleComp->BodyInstance.bLockXRotation = true;
    CapsuleComp->BodyInstance.bLockYRotation = true;
    CapsuleComp->SetLinearDamping(2.0f);

    // 비주얼: 메쉬
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메쉬는 충돌 없음
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemBase, CurrentState);
}

void AItemBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = EItemState::Basic;
}

void AItemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // [서버] 기본 상태일 때 Z축 강제 고정
    if (HasAuthority() && CurrentState == EItemState::Basic && bLockZAxis)
    {
        ApplyZLock();
    }

    // [클라이언트 & 리슨서버 호스트] 시각적 연출 업데이트
    UpdateVisuals(DeltaTime);
}

// --- Server Logics ---

void AItemBase::Server_ThrowTo_Implementation(FVector TargetPos, float Duration)
{
    ThrowStartPos = GetActorLocation();
    ThrowTargetPos = TargetPos;
    ThrowDuration = Duration;
    ThrowStartTime = GetWorld()->GetTimeSeconds();

    CurrentState = EItemState::Throwing;
    OnRep_CurrentState();
}

void AItemBase::Server_Spill_Implementation(FVector LandPosition)
{
    SetActorLocation(LandPosition);
    CurrentState = EItemState::Spilled;
    OnRep_CurrentState();
}

void AItemBase::Server_PickUp_Implementation(USceneComponent* ParentComp, FName SocketName)
{
    CapsuleComp->SetSimulatePhysics(false);
    AttachToComponent(ParentComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

    CurrentState = EItemState::Held;
    OnRep_CurrentState();
}

// --- Client Visuals (RepNotify) ---

void AItemBase::OnRep_CurrentState()
{
    switch (CurrentState)
    {
    case EItemState::Basic:
        bVisualDetached = false;
        VisualMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
        VisualMesh->SetSimulatePhysics(false);
        if (HasAuthority()) CapsuleComp->SetSimulatePhysics(true);
        break;

    case EItemState::Held:
    case EItemState::InCart: // [New] 카트 상태도 물리 끄기
        bVisualDetached = false;
        VisualMesh->SetSimulatePhysics(false);
        if (HasAuthority()) CapsuleComp->SetSimulatePhysics(false);
        break;

    case EItemState::Throwing:
        bVisualDetached = true;
        if (HasAuthority()) CapsuleComp->SetSimulatePhysics(false);
        ThrowStartTime = GetWorld()->GetTimeSeconds();
        break;

    case EItemState::Spilled:
        bVisualDetached = true;
        if (HasAuthority()) CapsuleComp->SetSimulatePhysics(false);

        VisualMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        VisualMesh->SetSimulatePhysics(true);

        FVector RandDir = FMath::VRand();
        RandDir.Z = FMath::Abs(RandDir.Z);
        VisualMesh->AddImpulse(RandDir * 500.0f + FVector(0, 0, 300.0f), NAME_None, true);
        break;
    }
}

void AItemBase::UpdateVisuals(float DeltaTime)
{
    // 1. [던지기] 포물선 이동 (예측)
    if (CurrentState == EItemState::Throwing)
    {
        float TimeElapsed = GetWorld()->GetTimeSeconds() - ThrowStartTime;
        float Alpha = FMath::Clamp(TimeElapsed / ThrowDuration, 0.0f, 1.0f);

        FVector NewPos = FMath::Lerp(ThrowStartPos, ThrowTargetPos, Alpha);
        float HeightCurve = 4.0f * ThrowHeight * Alpha * (1.0f - Alpha);
        NewPos.Z += HeightCurve;

        if (Alpha < 1.0f)
        {
            VisualMesh->SetWorldLocation(NewPos);
        }
        else
        {
            if (HasAuthority())
            {
                SetActorLocation(ThrowTargetPos);
                CurrentState = EItemState::Basic;
                OnRep_CurrentState();
            }
        }
    }
    // 2. [들기] 손에 들렸을 때 약간의 스프링 효과
    else if (CurrentState == EItemState::Held)
    {
        VisualMesh->SetRelativeLocation(FMath::VInterpTo(VisualMesh->GetRelativeLocation(), FVector::ZeroVector, DeltaTime, 10.0f));
    }
    // 3. [New] [카트 탑승] 관성 흔들림 (Fake Physics)
    else if (CurrentState == EItemState::InCart)
    {
        AActor* ParentActor = GetAttachParentActor();
        if (ParentActor)
        {
            // 부모(카트)의 속도를 로컬 공간으로 변환
            FVector Velocity = ParentActor->GetVelocity();
            // 부모의 Transform을 기준으로 World 속도를 Local 속도로 변환
            FVector LocalVel = ParentActor->GetActorTransform().InverseTransformVector(Velocity);

            // 속도에 비례한 목표 회전값 계산 (Drag 효과: 앞으로 가면 뒤로 젖혀짐)
            // X축 속도(전진) -> Y축 회전(Pitch) 
            // Y축 속도(횡이동) -> X축 회전(Roll)
            float TargetPitch = FMath::Clamp(-LocalVel.X * InCartSwayStrength, -30.0f, 30.0f);
            float TargetRoll = FMath::Clamp(LocalVel.Y * InCartSwayStrength, -30.0f, 30.0f);

            // 중요: Yaw(Z축 회전)는 소켓에 랜덤하게 꽂힌 초기 상태를 유지해야 자연스러움
            float CurrentYaw = VisualMesh->GetRelativeRotation().Yaw;

            FRotator TargetRot(TargetPitch, CurrentYaw, TargetRoll);

            // 현재 회전에서 목표 회전으로 부드럽게 보간 (RInterp)
            FRotator NewRot = FMath::RInterpTo(VisualMesh->GetRelativeRotation(), TargetRot, DeltaTime, InCartSwaySpeed);

            VisualMesh->SetRelativeRotation(NewRot);
        }
    }
}

void AItemBase::ApplyZLock()
{
    FVector Pos = GetActorLocation();
    if (FMath::Abs(Pos.Z) > 1.0f)
    {
        Pos.Z = 0.0f;
        SetActorLocation(Pos);
    }
}