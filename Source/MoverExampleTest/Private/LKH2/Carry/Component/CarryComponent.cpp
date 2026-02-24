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
  // // 원칙 준수: 부착(Attach) 및 물리/충돌 설정은 Manager와 ItemStateComponent에서 담당합니다.
  // 이 컴포넌트는 오직 '무엇을 들고 있는가'라는 포인터 상태만 리플리케이션을 통해 동기화합니다.
  // 클라이언트의 시각적 부착은 엔진의 AttachmentReplication을 통해 자동으로 수행됩니다.
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
    AActor* OldItem = CarriedActor;
    CarriedActor = nullptr;

    // 클라이언트 예측 혹은 서버 실행 모두 로컬 부착/탈착을 동기화해야 하므로 무조건 호출
    OnRep_CarriedActor(OldItem);
  }
}

void UCarryComponent::ForceEquip(AActor *ItemToEquip) {
  if (ItemToEquip && CarriedActor != ItemToEquip) {
    AActor* OldItem = CarriedActor;
    CarriedActor = ItemToEquip;

    // 클라이언트 예측 혹은 서버 실행 모두 로컬 부착/탈착을 동기화해야 하므로 무조건 호출
    OnRep_CarriedActor(OldItem);
  }
}

FCarryContext UCarryComponent::CreateCarryContext(AActor *Target,
                                                   ECarryInteractionType Type) const {
  FCarryContext Context(Cast<AActor>(GetOwner()), Type);
  Context.TargetActor = Target;
  
  // 손 상태 컨텍스트 채우기
  Context.bIsHandOccupied = (CarriedActor != nullptr);
  Context.InHandActor = CarriedActor;

  // 플레이어의 시야 방향이나 던지기 속도 등을 여기서 계산하여 컨텍스트에 담을 수 있음
  if (Type == ECarryInteractionType::Throw) {
    Context.Velocity = GetOwner()->GetActorForwardVector() * 1500.0f; // 기본값
  }

  return Context;
}

void UCarryComponent::ProcessInputBuffer(AActor *Target) {
  if (bWantsPickupOrDrop) {
    if (CarriedActor) {
      if (Target && Target->Implements<UCarryInterface>()) {
        ICarryInterface::Execute_OnCarryInteract(
            Target, CreateCarryContext(Target, ECarryInteractionType::Interact));
      } else {
        ICarryInterface::Execute_OnCarryInteract(
            CarriedActor, CreateCarryContext(nullptr, ECarryInteractionType::Interact));
        AActor *OldItem = CarriedActor;
        CarriedActor = nullptr;
        OnRep_CarriedActor(OldItem);
      }
    } else {
      if (Target && Target->Implements<UCarryInterface>()) {
        FCarryContext Context = CreateCarryContext(Target, ECarryInteractionType::Interact);
        bool bSuccess = ICarryInterface::Execute_OnCarryInteract(Target, Context);

        if (bSuccess && Cast<AItemBase>(Target) != nullptr) {
          AActor *OldItem = CarriedActor;
          CarriedActor = Target;
          OnRep_CarriedActor(OldItem);

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
      ICarryInterface::Execute_OnCarryInteract(
          CarriedActor, CreateCarryContext(nullptr, ECarryInteractionType::Throw));
    }
    AActor *OldItem = CarriedActor;
    CarriedActor = nullptr;
    OnRep_CarriedActor(OldItem);

    bWantsThrow = false;
    InputBufferTimer = 0.f;
  }
}
