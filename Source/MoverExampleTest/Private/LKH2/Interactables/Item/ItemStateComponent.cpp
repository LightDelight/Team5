// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

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
}

void UItemStateComponent::OnRep_ItemState() {
  // 클라이언트에서 상태 변경 시 시각적/물리적 처리
  SetItemState(CurrentState);
}

void UItemStateComponent::SetItemState(EItemState NewState) {
  CurrentState = NewState;

  AActor *Owner = GetOwner();
  if (!Owner)
    return;

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
    // 레벨에 놓여있으므로 서버-클라이언트 위치 동기화 필요
    Owner->SetReplicateMovement(true);
    break;
  case EItemState::Carried:
    if (RootPrim) {
      // 캐릭터가 들고 있으므로 아이템 자체 물리/충돌 끄기
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    // Carrier의 트랜스폼에 종속되므로 ReplicateMovement를 꺼서
    // 서버의 절대 좌표 갱신이 클라이언트의 로컬 부착 상태를 방해하지 않도록 합니다.
    Owner->SetReplicateMovement(false);
    break;
  case EItemState::Stored:
    if (RootPrim) {
      // 워크스테이션 같은 곳에 거치된 상태이므로 물리/충돌 끄기
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    // 부착 상태를 클라이언트에 동기화하기 위해 SetReplicateMovement를 끕니다.
    // (물리적 종속이 확실한 경우)
    Owner->SetReplicateMovement(false);
    break;
  }
}

void UItemStateComponent::ThrowItem(const FVector &Impulse) {
  // 상태 변경 및 물리 활성화
  SetItemState(EItemState::Dropped);

  if (Impulse.IsZero())
    return;

  if (AActor *Owner = GetOwner()) {
    if (UPrimitiveComponent *RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent())) {
      RootPrim->AddImpulse(Impulse, NAME_None, true);
    }
  }
}
