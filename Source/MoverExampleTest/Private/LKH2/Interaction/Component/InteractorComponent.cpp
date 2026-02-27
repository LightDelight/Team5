// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "LKH2/Grid/GridManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"

UInteractorComponent::UInteractorComponent() {
  SetIsReplicatedByDefault(true);
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled =
      false; // 세부 Trace 및 메시지 발송 평소 Off

  DetectionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
  DetectionSphere->SetupAttachment(this);
  DetectionSphere->InitSphereRadius(200.0f); // 감지 반경 임의 설정
  DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

  // 여기서 에디터에서 설정한(InteractionTraceChannel) Collision Profile 채널에만 Overlap 되도록 설정
  DetectionSphere->SetCollisionResponseToChannel(InteractionTraceChannel, ECR_Overlap);

  BufferedIntentTag = FGameplayTag::EmptyTag;
  InputBufferTimer = 0.f;
}

void UInteractorComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UInteractorComponent::BeginPlay() {
  Super::BeginPlay();

  PropertyComponent = GetOwner() ? GetOwner()->FindComponentByClass<UInteractorPropertyComponent>() : nullptr;

  // 에디터(블루프린트 등)에서 채널 변경을 지원하기 위해 BeginPlay에서도 한번 덮어씌웁니다.
  DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
  DetectionSphere->SetCollisionResponseToChannel(InteractionTraceChannel, ECR_Overlap);

  DetectionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &UInteractorComponent::OnDetectionBeginOverlap);
  DetectionSphere->OnComponentEndOverlap.AddDynamic(
      this, &UInteractorComponent::OnDetectionEndOverlap);

  // GridTarget 폴링 타이머 시작 (Tick 대신 낮은 비용으로 주기적 검사 수행)
  if (UWorld* World = GetWorld()) {
    World->GetTimerManager().SetTimer(GridTargetTimerHandle, this, &UInteractorComponent::PollGridTarget, GridTargetPollingInterval, true);
  }
}

void UInteractorComponent::OnDetectionBeginOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (OtherActor && OtherActor->Implements<UInteractionContextInterface>()) {
    OverlappingActors.AddUnique(OtherActor);

    // 대상 액터가 감지되었을 경우만 세부 Trace 및 메시지 발송 기능 ON (Tick
    // 켜기)
    SetComponentTickEnabled(true);
  }
}

void UInteractorComponent::OnDetectionEndOverlap(
    UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  if (OtherActor) {
    OverlappingActors.Remove(OtherActor);

    // 대상이 범위를 벗어나면 아웃라인 리셋
    if (CurrentSphereTarget == OtherActor) {
      SetSphereTarget(nullptr);
    }

    if (OverlappingActors.IsEmpty()) {
      // 모든 대상이 빠져나갔으므로 기능을 다시 Off (GridTarget은 무관한 타이머로 동작하므로 여기서는 Sphere 처리만 끄면 됨)
      SetComponentTickEnabled(false); 
    }
  }
}

void UInteractorComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // 입력 버퍼링 시간 업데이트
  if (InputBufferTimer > 0.f) {
    InputBufferTimer -= DeltaTime;
    if (InputBufferTimer <= 0.f) {
      BufferedIntentTag = FGameplayTag::EmptyTag;
    }
  }

  UpdateTargetInteractable();
  ProcessInputBuffer();
}

void UInteractorComponent::UpdateTargetInteractable()
{
  AActor *BestSphereTarget = nullptr;
  float MinDist = MAX_FLT;
  FVector MyLoc = GetComponentLocation();

  AActor* CurrentCarriedActor = PropertyComponent ? PropertyComponent->GetCarriedActor() : nullptr;

  // 1. (Track 1) Sphere 기반 탐색
  // 매 틱마다 태그를 검사하는 것은 비효율적이므로, 들고 있는 액터가 변경되었을 때만 검사 결과를 캐싱합니다.
  if (CachedCarriedActorForTagCheck != CurrentCarriedActor)
  {
    CachedCarriedActorForTagCheck = CurrentCarriedActor;
    bCachedShouldSearchSphere = true;

    if (bEnableSphereTraceWhenHoldingItem && CurrentCarriedActor) 
    {
      bCachedShouldSearchSphere = false; // 기본적으로 물건 들면 끔
      if (ILogicContextInterface* LogicInterface = Cast<ILogicContextInterface>(CurrentCarriedActor))
      {
        if (const FItemStatValue* Stat = LogicInterface->FindStat(RequiredSphereTraceTag))
        {
          bCachedShouldSearchSphere = true; // 요구 태그가 있으면 켬
        }
      }
    }
  }

  if (bCachedShouldSearchSphere) {
    for (AActor *Actor : OverlappingActors) {
      if (Actor && Actor != CurrentCarriedActor) { // 들고 있는 아이템은 제외
        float Dist = FVector::Distance(MyLoc, Actor->GetActorLocation());
        if (Dist < MinDist) {
          MinDist = Dist;
          BestSphereTarget = Actor;
        }
      }
    }
  }

  if (CurrentSphereTarget != BestSphereTarget) {
    SetSphereTarget(BestSphereTarget);
  }

}

void UInteractorComponent::PollGridTarget()
{
  if (!CachedGridManager) {
    if (UWorld* World = GetWorld()) {
      if (AGameStateBase* GameState = World->GetGameState()) {
        CachedGridManager = GameState->FindComponentByClass<UGridManagerComponent>();
      }
    }
  }

  AActor* BestGridTarget = nullptr;
  if (CachedGridManager && GetOwner()) {
    AActor* CurrentCarriedActor = PropertyComponent ? PropertyComponent->GetCarriedActor() : nullptr;
    FVector MyLoc = GetComponentLocation();

    // 플레이어의 위치 + 전방 벡터 방향으로 일정 거리만큼 떨어진 그리드를 계산
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector CheckLocation = MyLoc + (ForwardVector * GridTargetCheckDistance);
    
    BestGridTarget = CachedGridManager->GetActorAtWorldLocation(CheckLocation);

    // 자신이나 들고 있는 아이템, 상호작용할 수 없는 대상은 제외
    if (BestGridTarget == GetOwner() || BestGridTarget == CurrentCarriedActor ||
       (BestGridTarget && !BestGridTarget->Implements<UInteractionContextInterface>())) {
      BestGridTarget = nullptr;
    }
  }

  if (CurrentGridTarget != BestGridTarget) {
    SetGridTarget(BestGridTarget);
  }
}

void UInteractorComponent::SetSphereTarget(AActor *NewTarget) {
  bool bIsLocalPlayer = false;
  if (APawn *OwnerPawn = Cast<APawn>(GetOwner())) {
    bIsLocalPlayer = OwnerPawn->IsLocallyControlled();
  }

  if (CurrentSphereTarget != nullptr &&
      CurrentSphereTarget->Implements<UInteractionContextInterface>()) {
    if (bIsLocalPlayer && CurrentSphereTarget != CurrentGridTarget) {
      IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentSphereTarget, false);
    }
  }

  CurrentSphereTarget = NewTarget;

  if (CurrentSphereTarget != nullptr &&
      CurrentSphereTarget->Implements<UInteractionContextInterface>()) {
    if (bIsLocalPlayer) {
      IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentSphereTarget, true);
    }
  }
}

void UInteractorComponent::SetGridTarget(AActor *NewTarget) {
  bool bIsLocalPlayer = false;
  if (APawn *OwnerPawn = Cast<APawn>(GetOwner())) {
    bIsLocalPlayer = OwnerPawn->IsLocallyControlled();
  }

  if (CurrentGridTarget != nullptr &&
      CurrentGridTarget->Implements<UInteractionContextInterface>()) {
    if (bIsLocalPlayer && CurrentGridTarget != CurrentSphereTarget) {
      IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentGridTarget, false);
    }
  }

  CurrentGridTarget = NewTarget;

  if (CurrentGridTarget != nullptr &&
      CurrentGridTarget->Implements<UInteractionContextInterface>()) {
    if (bIsLocalPlayer) {
      IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentGridTarget, true);
    }
  }
}

void UInteractorComponent::TryInteract(FGameplayTag IntentTag) {
  BufferedIntentTag = IntentTag;
  InputBufferTimer = InputBufferTimeLimit;
  
  // 로컬 예측(ProcessInputBuffer) 과정에서 타겟의 물리가 꺼지며 Overlap이 해제되어
  // CurrentSphereTarget이 변경될 수 있으므로, 실행 전에 미리 보낼 타겟을 캐싱해 둡니다.
  AActor* BestTarget = CurrentSphereTarget ? CurrentSphereTarget.Get() : CurrentGridTarget.Get();

  ProcessInputBuffer();

  if (GetOwner() && !GetOwner()->HasAuthority()) {
    // 클라이언트에서 결정한 우선순위 타겟(SphereTarget 혹은 GridTarget)을 서버로 전송
    Server_TryInteract(BestTarget, IntentTag);
  }
}

bool UInteractorComponent::Server_TryInteract_Validate(AActor* Target, FGameplayTag IntentTag) {
  return true;
}

void UInteractorComponent::Server_TryInteract_Implementation(AActor* Target, FGameplayTag IntentTag) {
  BufferedIntentTag = IntentTag;
  InputBufferTimer = InputBufferTimeLimit;
  
  // 클라이언트가 결정해서 보낸 타겟을 즉시 강제 우선순위로 집어넣어 처리
  ProcessInputBuffer(Target);
}

FInteractionContext UInteractorComponent::CreateInteractionContext(AActor *Target,
                                                   FGameplayTag InteractionTag) const {
  FInteractionContext Context(Cast<AActor>(GetOwner()), InteractionTag);
  Context.TargetActor = Target;
  
  // 시도자 측 컴포넌트 세팅
  Context.InteractorComp = const_cast<UInteractorComponent*>(this);
  Context.InteractorPropertyComp = PropertyComponent;

  return Context;
}

void UInteractorComponent::ProcessInputBuffer(AActor* ServerOverrideTarget) {
  if (BufferedIntentTag == FGameplayTag::EmptyTag) {
    return;
  }

  AActor* CurrentCarriedActor = PropertyComponent ? PropertyComponent->GetCarriedActor() : nullptr;
  bool bSuccess = false;

  // 우선순위 1순위: ServerOverrideTarget(클라이언트가 강제한 타겟), 2순위: Sphere Target, 3순위: Grid Target
  AActor* PrimaryTarget = ServerOverrideTarget ? ServerOverrideTarget : CurrentSphereTarget.Get();
  AActor* SecondaryTarget = CurrentGridTarget.Get();

  // 클라이언트가 강제한 타겟이 없다면 기본 로컬 우선순위를 따름
  if (!ServerOverrideTarget) {
     PrimaryTarget = CurrentSphereTarget.Get();
     SecondaryTarget = CurrentGridTarget.Get();
  } else {
     // 오버라이드 타겟이 지정되었다면 보조 타겟 탐색은 무시 (원격 조준의 확정성)
     SecondaryTarget = nullptr;
  }

  if (CurrentCarriedActor) {
    // 들고 있는 아이템이 있을 때
    if (PrimaryTarget && PrimaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          PrimaryTarget, CreateInteractionContext(PrimaryTarget, BufferedIntentTag));
    }
    
    if (!bSuccess && SecondaryTarget && SecondaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          SecondaryTarget, CreateInteractionContext(SecondaryTarget, BufferedIntentTag));
    }

    // 타겟에게 처리를 못했다면, 들고 있는 아이템에게 전달
    if (!bSuccess && CurrentCarriedActor->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          CurrentCarriedActor, CreateInteractionContext(nullptr, BufferedIntentTag));
    }
  } else {
    // 들고 있는 아이템이 없을 때
    if (PrimaryTarget && PrimaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          PrimaryTarget, CreateInteractionContext(PrimaryTarget, BufferedIntentTag));
    }

    if (!bSuccess && SecondaryTarget && SecondaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          SecondaryTarget, CreateInteractionContext(SecondaryTarget, BufferedIntentTag));
    }
  }

  // 성공 여부에 따라 캐시 해제는 생략하거나 타겟 업데이트 시점에 리셋되도록 둠
  // SetTarget(nullptr) 대신, 상태가 변경되어 타겟 목록에서 빠지면 UpdateTargetInteractable에서 자동으로 리셋됨

  BufferedIntentTag = FGameplayTag::EmptyTag;
  InputBufferTimer = 0.f;
}
