// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interactables/Item/ItemSmoothingComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"

// Sets default values for this component's properties
UItemStateComponent::UItemStateComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  SetIsReplicatedByDefault(true);

  CurrentState = EItemState::Placed;
}

// Called when the game starts
void UItemStateComponent::BeginPlay() {
  Super::BeginPlay();

  // 초기 상태 로직 적용을 약간 지연시키거나 여기서 직접 호출해도 되지만, 
  // AItemBase의 BeginPlay에서 ObjectType이 설정된 이후여야 커스텀 채널이 유지됩니다.
  // Component의 BeginPlay가 Actor의 BeginPlay보다 먼저 호출될 수 있으므로, 
  // AItemBase쪽에서 주도하게 하는 것이 안전합니다. 일단 여기서 주석 처리합니다.
  // SetItemState(CurrentState); 
}

// Called every frame
void UItemStateComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UItemStateComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UItemStateComponent, CurrentState, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UItemStateComponent, RepParentActor, COND_None,
                                 REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UItemStateComponent, RepAttachComponent, COND_None,
                                 REPNOTIFY_Always);
}

void UItemStateComponent::OnRep_ItemState() {
  // 클라이언트에서 상태 변경 시 시각적/물리적 처리
  SetItemState(CurrentState);
}

void UItemStateComponent::SetItemState(EItemState NewState) {
  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  CurrentState = NewState;
  
  // 상태에 따른 물리 토글 및 캐리 컴포넌트 감지 여부(Collision Channel) 처리
  UPrimitiveComponent *RootPrim =
      Cast<UPrimitiveComponent>(Owner->GetRootComponent());

  switch (CurrentState) {
  case EItemState::Placed:
  case EItemState::Dropped:
    if (RootPrim) {
      // 물리 및 충돌 켜기
      RootPrim->SetSimulatePhysics(true);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    // 월드에 놓인 상태에서는 다시 스무딩을 켜서 지터를 방지합니다.
    if (UItemSmoothingComponent* SmoothingComp = Owner->FindComponentByClass<UItemSmoothingComponent>()) {
      SmoothingComp->SetSmoothingEnabled(true);
    }
    // 레벨에 놓여있으므로 서버-클라이언트 위치 동기화 필요
    Owner->SetReplicateMovement(true);
    // Carried 상태에서 NoCollision이었으로범위 안 DetectionSphere의
    // BeginOverlap이 발동하지 않을 수 있음. UpdateOverlaps로 강제 재계산.
    Owner->UpdateOverlaps();
    break;
  case EItemState::Carried:
  case EItemState::Stored:
    if (RootPrim) {
      // 캐릭터가 들고 있거나 보관된 상태이므로 아이템 자체 물리/충돌 끄기
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
      // 관성에 의한 이동 예측(Extrapolation)을 방지하기 위해 속도 초기화
      RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
      RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    // 부착 상태에서는 스무딩(Dead Reckoning) 설정을 켕니다. 
    // (실제 비주얼 분리는 Smoothing 컴포넌트 내부에서 Simulated Proxy인 경우에만 작동함)
    if (UItemSmoothingComponent* SmoothingComp = Owner->FindComponentByClass<UItemSmoothingComponent>()) {
      SmoothingComp->SetSmoothingEnabled(true);
    }
    // 부착된 상태에서도 위치 동기화(Attachment Replication 포함)가 원활하도록 ReplicateMovement를 유지합니다.
    Owner->SetReplicateMovement(true);
    break;
  case EItemState::Display:
    // 전시 상태: 스냅되어 있으므로 물리는 끄되, 상호작용(오버랩/트레이스)이 가능하도록 충돌은 켭니다.
    if (RootPrim) {
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      
      RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
      RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    if (UItemSmoothingComponent* SmoothingComp = Owner->FindComponentByClass<UItemSmoothingComponent>()) {
      SmoothingComp->SetSmoothingEnabled(true);
    }
    Owner->SetReplicateMovement(true);
    Owner->UpdateOverlaps();
    break;
  case EItemState::Spilled:
    if (RootPrim) {
      // 얭취로서 전복 시 취야하는 동작
      // 물리 켜기: 로컬 시뮬레이션으로 굴마다님
      RootPrim->SetSimulatePhysics(true);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      // 초경량 설정: 진짜 아이템에 물리 영향을 주지 않도록
      RootPrim->SetMassScale(NAME_None, 0.001f);
      // 관성 속도 초기화
      RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
      RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    // 아웃라인 비활성화: Spilled 아이템은 상호작용 불가이므로 시각적 피드백 제거
    // (클라이언트에서도 OnRep_ItemState로 호출되므로 서버-클라이언트 동일 동작)
    if (UInteractableComponent* IC = Owner->FindComponentByClass<UInteractableComponent>())
    {
      IC->SetOutlineEnabled(false, 2);
    }
    // 스무딩 끄기: 로컬에서만 연산하므로 서버 보간 불필요
    if (UItemSmoothingComponent* SmoothingComp = Owner->FindComponentByClass<UItemSmoothingComponent>()) {
      SmoothingComp->SetSmoothingEnabled(false);
    }
    // 위치 복제 끊기: 클라이언트마다 독립적으로 굴리도록
    Owner->SetReplicateMovement(false);
    break;
  }
}

void UItemStateComponent::ThrowItem(const FVector &Impulse) {
  // 상태 변경 및 물리 활성화
  SetItemState(EItemState::Dropped);

  // 부착 해제 정보 갱신
  UpdateAttachmentReplication();

  if (Impulse.IsZero())
    return;

  if (AActor *Owner = GetOwner()) {
    if (UPrimitiveComponent *RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent())) {
      RootPrim->AddImpulse(Impulse, NAME_None, true);
    }
  }
}

void UItemStateComponent::UpdateAttachmentReplication() {
  if (GetOwnerRole() != ROLE_Authority)
    return;

  AActor *OwnerActor = GetOwner();
  if (!OwnerActor)
    return;

  RepParentActor = OwnerActor->GetAttachParentActor();
  if (USceneComponent* Root = OwnerActor->GetRootComponent())
  {
      RepAttachComponent = Root->GetAttachParent();
  }
  else
  {
      RepAttachComponent = nullptr;
  }
}

void UItemStateComponent::OnRep_AttachmentData() {
  AActor *OwnerActor = GetOwner();
  if (!OwnerActor)
    return;

  if (RepParentActor) {
    // 클라이언트 강제 부착
    FAttachmentTransformRules Rules =
        FAttachmentTransformRules::SnapToTargetNotIncludingScale;

    if (RepAttachComponent) {
      OwnerActor->AttachToComponent(RepAttachComponent, Rules);
    } else {
      OwnerActor->AttachToActor(RepParentActor, Rules);
    }
  } else {
    // 부모가 없으면 분리 (월드 위치 유지)
    OwnerActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
  }
}
