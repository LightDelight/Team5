// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Carry/Component/CarryComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Item/ItemBase.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

UCarryComponent::UCarryComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled =
      false; // 세부 Trace 및 메시지 발송 평소 Off

  DetectionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
  DetectionSphere->SetupAttachment(this);
  DetectionSphere->InitSphereRadius(200.0f); // 감지 반경 임의 설정
  DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

  // 여기서 PhysicsBody 혹은 Item에 해당하는 Collision Profile 채널에만 Overlap
  // 되도록 설정해야 함
  DetectionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);

  bWantsPickupOrDrop = false;
  bWantsThrow = false;
  InputBufferTimer = 0.f;
}

void UCarryComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UCarryComponent, CarriedActor);
}

void UCarryComponent::OnRep_CarriedActor(AActor *OldCarriedActor) {
  // 이전 아이템 탈착 (워크스테이션 배치를 방해하지 않도록, 여전히 내 캐릭터에
  // 붙어있을 때만 탈착) 물리/충돌 관리는 온전히 ItemStateComponent 혹은 해당
  // 로직 모듈에게 위임합니다.
  if (OldCarriedActor && OldCarriedActor != CarriedActor) {
    if (OldCarriedActor->GetAttachParentActor() == GetOwner()) {
      OldCarriedActor->DetachFromActor(
          FDetachmentTransformRules::KeepWorldTransform);
    }
  }

  // 새로 들게 된 아이템 강제 부착 및 물리 끄기
  // (서버의 지연된 ReplicatedMovement 패킷이 클라이언트 예측을 덮어써서 물리
  // 버그로 인해 손에서 떨어지는 현상 원천 차단)
  if (CarriedActor) {
    if (UPrimitiveComponent *RootPrim =
            Cast<UPrimitiveComponent>(CarriedActor->GetRootComponent())) {
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
      RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
      RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }

    // 강제로 플레이어의 CarryComponent(손)에 명시적 스냅 부착
    CarriedActor->AttachToComponent(
        this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  }
}

void UCarryComponent::BeginPlay() {
  Super::BeginPlay();

  DetectionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &UCarryComponent::OnDetectionBeginOverlap);
  DetectionSphere->OnComponentEndOverlap.AddDynamic(
      this, &UCarryComponent::OnDetectionEndOverlap);
}

void UCarryComponent::OnDetectionBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (OtherActor && OtherActor->Implements<UCarryInterface>()) {
    OverlappingActors.AddUnique(OtherActor);

    // 대상 액터가 감지되었을 경우만 세부 Trace 및 메시지 발송 기능 ON (Tick
    // 켜기)
    SetComponentTickEnabled(true);
  }
}

void UCarryComponent::OnDetectionEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  if (OtherActor) {
    OverlappingActors.Remove(OtherActor);

    // 대상이 범위를 벗어나면 아웃라인 리셋
    if (CurrentTarget == OtherActor) {
      SetTarget(nullptr);
    }

    if (OverlappingActors.IsEmpty()) {
      // 모든 대상이 빠져나갔으므로 기능을 다시 Off
      SetComponentTickEnabled(false);
    }
  }
}

void UCarryComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // 입력 버퍼링 시간 업데이트
  if (InputBufferTimer > 0.f) {
    InputBufferTimer -= DeltaTime;
    if (InputBufferTimer <= 0.f) {
      bWantsPickupOrDrop = false;
      bWantsThrow = false;
    }
  }

  UpdateTargetCarryable();
  ProcessInputBuffer(CurrentTarget);
}

void UCarryComponent::UpdateTargetCarryable() {
  AActor *BestTarget = nullptr;
  float MinDist = MAX_FLT;
  FVector MyLoc = GetComponentLocation();

  // 가장 가까운(또는 시야각 기반으로 가장 적합한) 액터를 찾음
  for (AActor *Actor : OverlappingActors) {
    if (Actor && Actor != CarriedActor) { // 들고 있는 아이템은 제외
      float Dist = FVector::Distance(MyLoc, Actor->GetActorLocation());
      if (Dist < MinDist) {
        MinDist = Dist;
        BestTarget = Actor;
      }
    }
  }

  if (CurrentTarget != BestTarget) {
    // 이전 액터와 다르면 OFF 후 새 액터 ON 처리
    SetTarget(BestTarget);
  }
}

void UCarryComponent::SetTarget(AActor *NewTarget) {
  // 아웃라인(외곽선) 연산은 로컬 플레이어에게만 보이도록 제한
  bool bIsLocalPlayer = false;
  if (APawn *OwnerPawn = Cast<APawn>(GetOwner())) {
    bIsLocalPlayer = OwnerPawn->IsLocallyControlled();
  }

  if (CurrentTarget != nullptr &&
      CurrentTarget->Implements<UCarryInterface>()) {
    if (bIsLocalPlayer) {
      ICarryInterface::Execute_SetOutlineEnabled(CurrentTarget, false);
    }
  }

  CurrentTarget = NewTarget;

  if (CurrentTarget != nullptr &&
      CurrentTarget->Implements<UCarryInterface>()) {
    if (bIsLocalPlayer) {
      ICarryInterface::Execute_SetOutlineEnabled(CurrentTarget, true);
    }
  }
}

void UCarryComponent::TryPickupOrDrop() {
  bWantsPickupOrDrop = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(CurrentTarget);

  if (GetOwner() && !GetOwner()->HasAuthority()) {
    Server_TryPickupOrDrop(CurrentTarget);
  }
}

bool UCarryComponent::Server_TryPickupOrDrop_Validate(AActor *Target) {
  return true;
}
void UCarryComponent::Server_TryPickupOrDrop_Implementation(AActor *Target) {
  bWantsPickupOrDrop = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(Target); // 클라이언트가 알려준 타겟으로 실행
}

void UCarryComponent::TryThrow() {
  bWantsThrow = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(CurrentTarget);

  if (GetOwner() && !GetOwner()->HasAuthority()) {
    Server_TryThrow(CurrentTarget);
  }
}

bool UCarryComponent::Server_TryThrow_Validate(AActor *Target) { return true; }
void UCarryComponent::Server_TryThrow_Implementation(AActor *Target) {
  bWantsThrow = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(Target);
}

AActor *UCarryComponent::GetCarriedActor() const { return CarriedActor; }

void UCarryComponent::ForceDrop() {
  if (CarriedActor) {
    AActor* ActorToDrop = CarriedActor;
    CarriedActor = nullptr; // 재귀 방지: 먼저 참조를 해제
    if (ActorToDrop->Implements<UCarryInterface>()) {
      // Multicast RPC를 중복해서 부르지 않고 로컬하게 상호작용 처리
      ICarryInterface::Execute_OnCarryInteract(ActorToDrop, GetOwner(),
                                               ECarryInteractionType::Interact);
    }
  }
}

void UCarryComponent::ForceEquip(AActor *ItemToEquip) {
  if (ItemToEquip && !CarriedActor) {
    if (ItemToEquip->Implements<UCarryInterface>()) {
      // 위 로직과 동일하게 순수 로컬 실행 제어
      ICarryInterface::Execute_OnCarryInteract(ItemToEquip, GetOwner(),
                                               ECarryInteractionType::Interact);
      CarriedActor = ItemToEquip;
    }
  }
}

void UCarryComponent::ProcessInputBuffer(AActor *Target) {
  if (bWantsPickupOrDrop) {
    if (CarriedActor) {
      if (Target && Target->Implements<UCarryInterface>()) {
        ICarryInterface::Execute_OnCarryInteract(
            Target, GetOwner(), ECarryInteractionType::Interact);
      } else {
        ICarryInterface::Execute_OnCarryInteract(
            CarriedActor, GetOwner(), ECarryInteractionType::Interact);
        CarriedActor = nullptr;
      }
    } else {
      if (Target && Target->Implements<UCarryInterface>()) {
        bool bSuccess = ICarryInterface::Execute_OnCarryInteract(
            Target, GetOwner(), ECarryInteractionType::Interact);

        if (bSuccess && Cast<AItemBase>(Target) != nullptr) {
          CarriedActor = Target;
          SetTarget(nullptr);
        }
      }
    }

    bWantsPickupOrDrop = false;
    InputBufferTimer = 0.f;
  }

  if (bWantsThrow && CarriedActor) {
    if (CarriedActor->Implements<UCarryInterface>()) {
      // 던지기 처리 (로직 모듈에 위임)
      ICarryInterface::Execute_OnCarryInteract(CarriedActor, GetOwner(),
                                               ECarryInteractionType::Throw);
    }
    CarriedActor = nullptr;
    bWantsThrow = false;
    InputBufferTimer = 0.f;
  }
}
