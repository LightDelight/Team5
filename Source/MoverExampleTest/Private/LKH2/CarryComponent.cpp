// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/CarryComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/CarryableInterface.h"
#include "LKH2/ItemBase.h"
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

void UCarryComponent::OnRep_CarriedActor() {
  // 클라이언트 측에서 CarriedActor가 변했을 때(예: 늦게 입장한 클라이언트 보정)
  if (CarriedActor && CarriedActor->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_OnPickedUp(CarriedActor, GetOwner());
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
  if (OtherActor && OtherActor->Implements<UCarryableInterface>()) {
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
  if (CurrentTarget != nullptr &&
      CurrentTarget->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_SetOutlineEnabled(CurrentTarget, false);
  }

  CurrentTarget = NewTarget;

  if (CurrentTarget != nullptr &&
      CurrentTarget->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_SetOutlineEnabled(CurrentTarget, true);
  }
}

void UCarryComponent::TryPickupOrDrop() {
  if (GetOwner() && !GetOwner()->HasAuthority()) {
    Server_TryPickupOrDrop(CurrentTarget);
    return; // 서버로 현재 클라이언트의 타겟을 보냄
  }

  // 서버거나 호스트면 바로 실행
  bWantsPickupOrDrop = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(CurrentTarget);
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
  if (GetOwner() && !GetOwner()->HasAuthority()) {
    Server_TryThrow(CurrentTarget);
    return;
  }

  bWantsThrow = true;
  InputBufferTimer = InputBufferTimeLimit;
  ProcessInputBuffer(CurrentTarget);
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
    if (CarriedActor->Implements<UCarryableInterface>()) {
      Multicast_OnDropped(CarriedActor);
    }
    CarriedActor = nullptr;
  }
}

void UCarryComponent::ForceEquip(AActor *ItemToEquip) {
  if (ItemToEquip && !CarriedActor) {
    if (ItemToEquip->Implements<UCarryableInterface>()) {
      Multicast_OnPickedUp(ItemToEquip);
      CarriedActor = ItemToEquip;
    }
  }
}

void UCarryComponent::ProcessInputBuffer(AActor *Target) {
  if (bWantsPickupOrDrop) {
    if (CarriedActor) {
      // 1. 이미 아이템을 들고 있는 상태
      if (Target && Target->Implements<UCarryableInterface>()) {
        // 1-A. 워크스테이션 같은 상호작용 가능한 대상(빈 공간이 아님)을
        // 바라보고 있음 Target 쪽에 먼저 상호작용 트리거 (워크스테이션의
        // OnPickedUp은 OnInteract를 내부 호출함)
        Multicast_OnPickedUp(Target);
      } else {
        // 1-B. 허공이거나 상호작용 불가능한 곳이면 그냥 바닥에 떨어뜨림
        Multicast_OnDropped(CarriedActor);
        CarriedActor = nullptr;
      }
    } else {
      // 2. 빈 손인 상태 (주울 대상이나 상호작용 대상이 있는지 확인)
      if (Target && Target->Implements<UCarryableInterface>()) {
        Multicast_OnPickedUp(Target);

        // 워크스테이션이 아닌 상호작용 가능한 순수 아이템(AItemBase)일 때만
        // 손에 쥠
        if (Cast<AItemBase>(Target) != nullptr) {
          CarriedActor = Target;
          SetTarget(nullptr); // 성공적으로 집었으면 아웃라인 끄기
        }
      }
    }

    bWantsPickupOrDrop = false;
    InputBufferTimer = 0.f;
  }

  if (bWantsThrow && CarriedActor) {
    if (CarriedActor->Implements<UCarryableInterface>()) {
      FVector ThrowVelocity = GetForwardVector() * 800.0f +
                              FVector(0, 0, 300.0f); // 임의의 던지는 힘
      Multicast_OnThrown(CarriedActor, ThrowVelocity);
    }
    CarriedActor = nullptr;
    bWantsThrow = false;
    InputBufferTimer = 0.f;
  }
}

void UCarryComponent::Multicast_OnPickedUp_Implementation(AActor *Item) {
  if (Item && Item->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_OnPickedUp(Item, GetOwner());
  }
}

void UCarryComponent::Multicast_OnDropped_Implementation(AActor *Item) {
  if (Item && Item->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_OnDropped(Item);
  }
}

void UCarryComponent::Multicast_OnThrown_Implementation(AActor *Item,
                                                        FVector ThrowVelocity) {
  if (Item && Item->Implements<UCarryableInterface>()) {
    ICarryableInterface::Execute_OnThrown(Item, ThrowVelocity);
  }
}
