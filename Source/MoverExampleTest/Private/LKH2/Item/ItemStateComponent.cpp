// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Item/ItemStateComponent.h"
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

  SetItemState(CurrentState); // 초기 상태 로직 적용
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
      RootPrim->SetCollisionProfileName(TEXT("PhysicsActor"));
    }
    // 레벨에 놓여있으므로 서버-클라이언트 위치 동기화 필요
    Owner->SetReplicateMovement(true);
    break;
  case EItemState::Carried:
    if (RootPrim) {
      // 캐릭터가 들고 있으므로 아이템 자체 물리/충돌 끄기
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
    }
    // Carrier의 트랜스폼에 종속되더라도 ReplicateMovement가 꺼져있으면
    // 서버에서 AttachToActor 된 사실 자체가 클라이언트로 넘어가지 않으므로 true
    // 유지.
    Owner->SetReplicateMovement(true);
    break;
  case EItemState::Stored:
    if (RootPrim) {
      // 워크스테이션 같은 곳에 거치된 상태이므로 물리/충돌 끄기
      RootPrim->SetSimulatePhysics(false);
      RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
    }
    // 부착 상태를 클라이언트에 동기화하기 위해 유지
    Owner->SetReplicateMovement(true);
    break;
  }
}
