// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemSmoothingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interactables/Item/ItemBase.h"

// Sets default values for this component's properties
UItemSmoothingComponent::UItemSmoothingComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
  PrimaryComponentTick.TickGroup = TG_PostPhysics; // 물리 처리 후 보간
  bAutoActivate = true;
  SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UItemSmoothingComponent::BeginPlay() { 
  Super::BeginPlay(); 
  
  // 틱 명시적 활성화 시도
  SetComponentTickEnabled(true);
}

void UItemSmoothingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(UItemSmoothingComponent, bUseDeadReckoning);
}

// Called every frame
void UItemSmoothingComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // 로컬에서 직접 제어하는 상황(자신이 들고 있는 상태)이 아니면 스무딩 보간을 수행함
  bool bIsLocallyControlled = false;
  if (AActor* MyOwner = GetOwner()) {
    // 1. 소유권 역할 체크 (클라이언트)
    if (MyOwner->GetLocalRole() == ROLE_AutonomousProxy) {
      bIsLocallyControlled = true;
    }
    // 2. 서버(Listen Host)인 경우 최상위 부모(Pawn)의 로컬 제어 여부 확인
    else if (MyOwner->HasAuthority() && MyOwner->GetNetMode() != NM_DedicatedServer) {
      AActor* RootOwner = MyOwner;
      while (RootOwner->GetAttachParentActor()) {
        RootOwner = RootOwner->GetAttachParentActor();
      }
      if (APawn* P = Cast<APawn>(RootOwner)) {
        bIsLocallyControlled = P->IsLocallyControlled();
      }
    }
  }

  if (bUseDeadReckoning && TargetRoot && InterpolatedVisual) {
    if (!bIsLocallyControlled) {
      FVector CurrentLoc = InterpolatedVisual->GetComponentLocation();
      FVector TargetLoc = TargetRoot->GetComponentLocation();

      FRotator CurrentRot = InterpolatedVisual->GetComponentRotation();
      FRotator TargetRotVal = TargetRoot->GetComponentRotation();

      FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime,
                                       LocationInterpSpeed);
      FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRotVal, DeltaTime,
                                        RotationInterpSpeed);

      InterpolatedVisual->SetWorldLocationAndRotation(NewLoc, NewRot);
    } else {
      // 로컬 제어(본인): 만약 비주얼이 아직 분리된 상태라면 즉시 재부착(Snap)
      if (InterpolatedVisual->GetAttachParent() != TargetRoot) {
        ApplySmoothingState();
      }
    }
  }
}

void UItemSmoothingComponent::InitialSetup(USceneComponent *InRoot,
                                           USceneComponent *InVisual) {
  TargetRoot = InRoot;
  InterpolatedVisual = InVisual;

  ApplySmoothingState();
}

void UItemSmoothingComponent::SetSmoothingEnabled(bool bEnabled) {
  if (bUseDeadReckoning == bEnabled)
    return;
  bUseDeadReckoning = bEnabled;

  ApplySmoothingState();
}

void UItemSmoothingComponent::OnRep_UseDeadReckoning() {
  ApplySmoothingState();
}
void UItemSmoothingComponent::ApplySmoothingState() {
  if (!InterpolatedVisual || !TargetRoot)
    return;

  // bUseDeadReckoning이 켜져 있더라도, 로컬에서 제어 중인 아이템(본인 화면)이면 비주얼 분리를 하지 않음
  bool bIsLocallyControlled = false;
  if (AActor* MyOwner = GetOwner()) {
    if (MyOwner->GetLocalRole() == ROLE_AutonomousProxy) bIsLocallyControlled = true;
    else if (MyOwner->HasAuthority() && MyOwner->GetNetMode() != NM_DedicatedServer) {
      AActor* RootOwner = MyOwner;
      while (RootOwner->GetAttachParentActor()) RootOwner = RootOwner->GetAttachParentActor();
      if (APawn* P = Cast<APawn>(RootOwner)) bIsLocallyControlled = P->IsLocallyControlled();
    }
  }
  
  bool bReallyUseSmoothing = bUseDeadReckoning && !bIsLocallyControlled;

  if (bReallyUseSmoothing) {
    // 스무딩 활성화 시 틱 켜기 및 비주얼 메쉬를 루트에서 분리 (월드에 남겨 지터 보정)
    SetComponentTickEnabled(true);
    InterpolatedVisual->DetachFromComponent(
        FDetachmentTransformRules::KeepWorldTransform);
  } else {
    // 소유자(Owner)인 경우: 틱 끄기 및 루트에 물리적으로 완벽하게 부착
    SetComponentTickEnabled(false);
    InterpolatedVisual->AttachToComponent(
        TargetRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    // 부착 후 relative location이 (0,0,0)이 되는데, ItemData에 정의된 값을 재적용합니다.
    if (AActor* Owner = GetOwner()) {
        if (AItemBase* Item = Cast<AItemBase>(Owner)) {
            Item->SetItemDataAndApply(Item->GetItemData());
        }
    }
  }
}
