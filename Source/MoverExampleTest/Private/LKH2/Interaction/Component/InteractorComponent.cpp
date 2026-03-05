// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Component/InteractorPropertyComponent.h"
#include "LKH2/Interactables/Item/ItemBase.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "LKH2/Grid/GridManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "LKH2/Interactables/Item/Manager/ItemManagerSubsystem.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interactables/Item/ItemBase.h"

static FString GetItemDataName(AActor* Actor)
{
  if (!Actor) return TEXT("None");

  if (AItemBase* Item = Cast<AItemBase>(Actor))
  {
    FGuid InstanceId = Item->GetInstanceId();
    if (InstanceId.IsValid())
    {
      return FString::Printf(TEXT("%s(%s)"), *Actor->GetName(), *InstanceId.ToString());
    }
  }
  return Actor->GetName();
}

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
  DOREPLIFETIME(UInteractorComponent, bIsWorking);
}

void UInteractorComponent::SetIsWorking(bool bWorking, AActor* InTargetActor, FGameplayTag InCancelTag)
{
  UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] SetIsWorking(%d) 호출됨. 현재 bIsWorking=%d, Owner=%s"),
    bWorking, bIsWorking, *GetOwner()->GetName());

  if (bIsWorking == bWorking) 
  {
    UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] SetIsWorking: 이미 같은 값이므로 조기 반환."));
    return;
  }
  bIsWorking = bWorking;

  if (bWorking)
  {
    // 작업 대상과 Cancel Intent 태그 저장
    WorkingTargetActor = InTargetActor;
    WorkingCancelIntentTag = InCancelTag;
  }
  else
  {
    // 작업 종료 시 초기화
    WorkingTargetActor = nullptr;
    WorkingCancelIntentTag = FGameplayTag();
  }

  // 이동 차단/해제
  if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
  {
    if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
    {
      if (bWorking)
      {
        UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] DisableMovement() 호출."));
        MoveComp->DisableMovement();
      }
      else
      {
        UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] SetMovementMode(MOVE_Walking) 호출."));
        MoveComp->SetMovementMode(MOVE_Walking);
      }
    }
    else
    {
      UE_LOG(LogTemp, Error, TEXT("[InteractorComp] SetIsWorking: CharacterMovementComponent를 찾을 수 없습니다!"));
    }
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("[InteractorComp] SetIsWorking: Owner가 ACharacter가 아닙니다!"));
  }
}

// 클라이언트에서 복제된 bIsWorking에 따라 이동 상태를 동기화
void UInteractorComponent::OnRep_IsWorking()
{
  if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
  {
    if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
    {
      if (bIsWorking)
      {
        MoveComp->DisableMovement();
      }
      else
      {
        MoveComp->SetMovementMode(MOVE_Walking);
      }
    }
  }
}


void UInteractorComponent::BeginPlay() {
  Super::BeginPlay();

  PropertyComponent = nullptr;
  if (AActor* OwnerActor = GetOwner())
  {
      TArray<UInteractorPropertyComponent*> PropComps;
      OwnerActor->GetComponents<UInteractorPropertyComponent>(PropComps);
      for (UInteractorPropertyComponent* Comp : PropComps)
      {
          if (Comp->GetName().Contains(TEXT("InteractorProperty")))
          {
              PropertyComponent = Comp;
              break;
          }
      }
      if (!PropertyComponent && PropComps.Num() > 0)
      {
          PropertyComponent = PropComps[0];
      }
  }

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
    // Spilled 아이템은 상호작용 대상에서 제외 (Replicated 상태 기반)
    if (AItemBase* AsItem = Cast<AItemBase>(OtherActor))
    {
      if (UItemStateComponent* StateComp = AsItem->FindComponentByClass<UItemStateComponent>())
      {
        if (StateComp->CurrentState == EItemState::Spilled)
          return;
      }
    }
    OverlappingActors.AddUnique(OtherActor);
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
    
    // 시야 기반 타겟팅을 위한 방향 계산
    FVector CameraLocation = MyLoc;
    FVector CameraForward = GetOwner()->GetActorForwardVector();
    
    // 플레이어 컨트롤러의 카메라(뷰포인트)가 있다면 그 시점을 사용
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
      if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
      {
        FRotator CameraRotation;
        PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
        CameraForward = CameraRotation.Vector();
      }
    }

    float BestScore = -MAX_FLT; // 점수 기반 시스템 (높을수록 좋음)

    for (AActor *Actor : OverlappingActors) {
      if (Actor && Actor != CurrentCarriedActor) {
        // Spilled 아이템은 타겟 대상에서 제외 (Replicated 상태 기반)
        if (AItemBase* AsItem = Cast<AItemBase>(Actor))
        {
          if (UItemStateComponent* StateComp = AsItem->FindComponentByClass<UItemStateComponent>())
          {
            if (StateComp->CurrentState == EItemState::Spilled)
              continue;
          }
        }
        
        FVector DirToTarget = (Actor->GetActorLocation() - CameraLocation);
        float Dist = DirToTarget.Size();
        
        // 정규화된 방향 벡터
        DirToTarget.Normalize();
        
        // 시선 방향과 타겟 방향의 내적 (Angle)
        float DotProduct = FVector::DotProduct(CameraForward, DirToTarget);
        
        // 거리 범위(최대 감지 제한) 확인
        if (Dist > 0.0f)
        {
          // 점수 = (시선의 일치도 가중치) - (거리 가중치)
          // 내적이 1.0(완벽히 쳐다봄)에 가까울수록 점수가 기하급수적으로 폭증하도록 설계 (시야 최우선)
          // 0.8 이하의 내적은 화면 가장자리이므로 점수를 대폭 깎음
          float AngleScore = FMath::Clamp(DotProduct, 0.0f, 1.0f) * 1000.f;  
          float DistPenalty = Dist * 0.5f; // 거리가 멀어질수록 감점
          
          float FinalScore = AngleScore - DistPenalty;

          // Hysteresis (깜빡임 방지 타겟 유지 보너스)
          // 기존 타겟에게는 추가 점수를 부여하여 점수가 비슷할 때 타겟이 요동치는 것을 방지합니다.
          if (Actor == CurrentSphereTarget.Get())
          {
            FinalScore += 50.0f; // 보너스 수치는 테스트하며 조정 가능
          }
          
          if (FinalScore > BestScore) {
            BestScore = FinalScore;
            BestSphereTarget = Actor;
          }
        }
      }
    }
  }

if (CurrentSphereTarget != BestSphereTarget)
{

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
    
    TArray<AActor*> NearbyActors = CachedGridManager->GetNearbyActors(CheckLocation, 1);
    
    float MinDistSq = MAX_FLT;
    
    for (AActor* Actor : NearbyActors) {
      // 자신이나 들고 있는 아이템, 상호작용할 수 없는 대상은 제외
      if (!Actor || Actor == GetOwner() || Actor == CurrentCarriedActor ||
          !Actor->Implements<UInteractionContextInterface>()) {
        continue;
      }
      
      float DistSq = FVector::DistSquared(MyLoc, Actor->GetActorLocation());
      if (DistSq < MinDistSq) {
        MinDistSq = DistSq;
        BestGridTarget = Actor;
      }
    }
  }

  if (CurrentGridTarget != BestGridTarget)
  {
    SetGridTarget(BestGridTarget);
  }
}

void UInteractorComponent::SetSphereTarget(AActor* NewTarget)
{
    TryCancelWorkingOnTargetChange(NewTarget);

    bool bIsLocalPlayer = false;
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        bIsLocalPlayer = OwnerPawn->IsLocallyControlled();

    if (CurrentSphereTarget != nullptr &&
        CurrentSphereTarget->Implements<UInteractionContextInterface>())
    {
        if (bIsLocalPlayer && CurrentSphereTarget != CurrentGridTarget)
        {
            IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentSphereTarget, false);
        }
    }

    CurrentSphereTarget = NewTarget;

    if (CurrentSphereTarget != nullptr &&
        CurrentSphereTarget->Implements<UInteractionContextInterface>())
    {
        if (bIsLocalPlayer)
        {
            IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentSphereTarget, true);
        }
    }
}

void UInteractorComponent::SetGridTarget(AActor* NewTarget)
{
    TryCancelWorkingOnTargetChange(NewTarget);

    bool bIsLocalPlayer = false;
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        bIsLocalPlayer = OwnerPawn->IsLocallyControlled();

    if (CurrentGridTarget != nullptr &&
        CurrentGridTarget->Implements<UInteractionContextInterface>())
    {
        if (bIsLocalPlayer && CurrentGridTarget != CurrentSphereTarget)
        {
            IInteractionContextInterface::Execute_SetOutlineEnabled(CurrentGridTarget, false);
        }
    }

    CurrentGridTarget = NewTarget;

    if (CurrentGridTarget != nullptr &&
        CurrentGridTarget->Implements<UInteractionContextInterface>())
    {
        if (bIsLocalPlayer)
        {
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

  UE_LOG(LogTemp, Log, TEXT("[InteractorComp] TryInteract: Intent=%s, SphereTarget=%s, GridTarget=%s, Authority=%s"),
    *IntentTag.ToString(),
    CurrentSphereTarget ? *GetItemDataName(CurrentSphereTarget.Get()) : TEXT("None"),
    CurrentGridTarget ? *GetItemDataName(CurrentGridTarget.Get()) : TEXT("None"),
    GetOwner()->HasAuthority() ? TEXT("Server") : TEXT("Client"));

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

  AActor* ActualInteractedTarget = nullptr;

  if (CurrentCarriedActor) {
    // 들고 있는 아이템이 있을 때
    if (PrimaryTarget && PrimaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          PrimaryTarget, CreateInteractionContext(PrimaryTarget, BufferedIntentTag));
      if (bSuccess) ActualInteractedTarget = PrimaryTarget;
    }
    
    if (!bSuccess && SecondaryTarget && SecondaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          SecondaryTarget, CreateInteractionContext(SecondaryTarget, BufferedIntentTag));
      if (bSuccess) ActualInteractedTarget = SecondaryTarget;
    }

    // 타겟에게 처리를 못했다면, 들고 있는 아이템에게 전달
    // BestTarget을 함께 전달하여 CarriedActor(Container 등)가 대상을 인식 가능
    if (!bSuccess && CurrentCarriedActor->Implements<UInteractionContextInterface>()) {
      AActor* BestTarget = PrimaryTarget ? PrimaryTarget : SecondaryTarget;
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          CurrentCarriedActor, CreateInteractionContext(BestTarget, BufferedIntentTag));
      if (bSuccess) ActualInteractedTarget = CurrentCarriedActor;
    }
  } else {
    // 들고 있는 아이템이 없을 때
    if (PrimaryTarget && PrimaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          PrimaryTarget, CreateInteractionContext(PrimaryTarget, BufferedIntentTag));
      if (bSuccess) ActualInteractedTarget = PrimaryTarget;
    }

    if (!bSuccess && SecondaryTarget && SecondaryTarget->Implements<UInteractionContextInterface>()) {
      bSuccess = IInteractionContextInterface::Execute_OnInteract(
          SecondaryTarget, CreateInteractionContext(SecondaryTarget, BufferedIntentTag));
      if (bSuccess) ActualInteractedTarget = SecondaryTarget;
    }
  }
  if (!bSuccess) {
    UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] 상호작용 실패 (모든 타겟에서 OnInteract가 false 반환됨)"));
  }

  BufferedIntentTag = FGameplayTag::EmptyTag;
  InputBufferTimer = 0.f;
}

void UInteractorComponent::TryCancelWorkingOnTargetChange(AActor* NewTarget)
{
  if (!bIsWorking) return;

  AActor* OldTarget = WorkingTargetActor.Get();
  if (!OldTarget || OldTarget == NewTarget) return;
  if (!WorkingCancelIntentTag.IsValid()) return;

  UE_LOG(LogTemp, Warning, TEXT("[InteractorComp] 작업 중 타겟 변경 감지! OldTarget=%s → NewTarget=%s, Cancel Intent 발송"),
    *OldTarget->GetName(), NewTarget ? *NewTarget->GetName() : TEXT("None"));

  // 기존 파이프라인을 통해 Cancel Intent를 이전 타겟에 발송
  if (OldTarget->Implements<UInteractionContextInterface>())
  {
    FInteractionContext CancelContext = CreateInteractionContext(OldTarget, WorkingCancelIntentTag);
    IInteractionContextInterface::Execute_OnInteract(OldTarget, CancelContext);
  }
}
