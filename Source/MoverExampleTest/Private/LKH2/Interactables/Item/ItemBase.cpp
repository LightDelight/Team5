// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Item/ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interactables/Item/ItemData.h"
#include "LKH2/Interactables/Item/ItemSmoothingComponent.h"
#include "LKH2/Interactables/Item/ItemStateComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Base/LogicBlackboard.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "Net/UnrealNetwork.h"

void AItemBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AItemBase, InstanceId);
  DOREPLIFETIME_CONDITION_NOTIFY(AItemBase, ItemData, COND_None,
                                 REPNOTIFY_Always);
}

void AItemBase::OnRep_ItemData() {
  SetItemDataAndApply(ItemData);
  if (InteractableComponent) {
    InteractableComponent->InitializeLogic(ItemData, this);
  }
  if (BlackboardComponent) {
    BlackboardComponent->MarkLogicInitialized(ItemData);
  }
}

UInteractableComponent *AItemBase::GetInteractableComponent() const {
  return InteractableComponent;
}

UInteractablePropertyComponent *AItemBase::GetPropertyComponent() const {
  return PropertyComponent;
}

FLogicBlackboard *AItemBase::GetLogicBlackboard() {
  return BlackboardComponent ? BlackboardComponent->GetBlackboard() : nullptr;
}

const FItemStatValue *AItemBase::FindStat(const FGameplayTag &Tag) const {
  return BlackboardComponent ? BlackboardComponent->FindStat(Tag) : nullptr;
}

void AItemBase::SetStat(const FGameplayTag &Tag, const FItemStatValue &Value) {
  if (BlackboardComponent) {
    BlackboardComponent->SetStat(Tag, Value);
  }
}

FGameplayTag AItemBase::ResolveKey(const FGameplayTag &Key) const {
  return BlackboardComponent ? BlackboardComponent->ResolveKey(Key) : Key;
}

// 기본값 설정
AItemBase::AItemBase() {
  // 이 액터가 프레임마다 Tick()을 호출하도록 설정합니다. 성능 향상을 위해 필요
  // 없다면 끌 수 있습니다.
  PrimaryActorTick.bCanEverTick = true;

  // 리플리케이션 설정
  bReplicates = true;
  SetReplicateMovement(true);

  // 물리 충돌체 생성 (Sphere Setup)
  SphereCollision =
      CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
  RootComponent = SphereCollision;

  // 물리 설정
  SphereCollision->InitSphereRadius(32.0f); // 기본 반지름 설정
  SphereCollision->SetSimulatePhysics(true);
  SphereCollision->SetCollisionProfileName(TEXT("PhysicsActor")); // 표준 물리 프로필 고정 (디폴트)
  SphereCollision->SetGenerateOverlapEvents(true);

  // 세팅: 밀리긴 하지만 튕기거나 미끄러지지는 않도록 (바운스 없음, 높은
  // 마찰/감쇠)
  SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  SphereCollision->SetCollisionResponseToAllChannels(ECR_Block);

  // 회전 잠금 Z축만 허용
  SphereCollision->BodyInstance.bLockXRotation = true;
  SphereCollision->BodyInstance.bLockYRotation = true;
  SphereCollision->BodyInstance.bLockZRotation =
      false; // 기본 보행처럼 회전 가능

  // 댐핑 설정으로 미끄럽지 않게
  SphereCollision->SetLinearDamping(1.0f);
  SphereCollision->SetAngularDamping(2.0f);

  // 시각적 메쉬 생성
  VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
  VisualMesh->SetupAttachment(SphereCollision);
  VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 시각 전용

  StateComponent =
      CreateDefaultSubobject<UItemStateComponent>(TEXT("StateComponent"));
  SmoothingComponent = CreateDefaultSubobject<UItemSmoothingComponent>(
      TEXT("SmoothingComponent"));
  InteractableComponent =
      CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
  BlackboardComponent = CreateDefaultSubobject<ULogicContextComponent>(
      TEXT("BlackboardComponent"));
}

// 게임이 시작되거나 스폰될 때 호출됩니다
void AItemBase::BeginPlay() {
  Super::BeginPlay();

  // 에디터에서 설정한 커스텀 오브젝트 타입 적용 및 오버랩 허용
  if (SphereCollision) {
    SphereCollision->SetCollisionObjectType(ObjectType);
    SphereCollision->SetCollisionResponseToChannel(ObjectType, ECR_Overlap);
  }

  if (HasAuthority() && ItemData) {
    if (InteractableComponent) {
      InteractableComponent->InitializeLogic(ItemData, this);
    }
    if (BlackboardComponent) {
      BlackboardComponent->MarkLogicInitialized(ItemData);
    }
  }

  if (SmoothingComponent) {
    SmoothingComponent->InitialSetup(SphereCollision, VisualMesh);
  }

  // 컴포넌트들의 BeginPlay보다 늦게 확실하게 커스텀 콜리전 채널 세팅 후 초기 상태 적용
  if (StateComponent) {
    StateComponent->SetItemState(StateComponent->CurrentState);
  }
}

bool AItemBase::OnInteract_Implementation(const FInteractionContext &Context) {
  if (!StateComponent || !InteractableComponent) {
    return false;
  }

  return InteractableComponent->OnInteract(Context);
}

void AItemBase::SetOutlineEnabled_Implementation(bool bEnabled) {
  if (InteractableComponent) {
    InteractableComponent->SetOutlineEnabled(bEnabled);
  }
}

void AItemBase::SetItemDataAndApply(UItemData *InData) {
  ItemData = InData;
  if (ItemData) {
    UStaticMesh *Mesh = ItemData->GetEffectiveItemMesh();
    if (Mesh && VisualMesh) {
      VisualMesh->SetStaticMesh(Mesh);
    }
    if (SphereCollision) {
      SphereCollision->SetMassOverrideInKg(NAME_None,
                                           ItemData->GetEffectiveWeight(),
                                           true);
      SphereCollision->SetSphereRadius(ItemData->GetEffectiveSphereRadius());
    }
    if (VisualMesh) {
      VisualMesh->SetRelativeLocation(
          ItemData->GetEffectiveMeshRelativeLocation());
    }

    // [Decoupling] 이제 컴포넌트들이 Pull 패턴을 사용하여 직접 조회하므로
    // 수동으로 전달(Push)할 필요가 없습니다.
  }
}

TArray<ULogicModuleBase *> AItemBase::GetLogicModules() const {
  return InteractableComponent ? InteractableComponent->GetLogicModules()
                               : TArray<ULogicModuleBase *>();
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);
  
  // 에디터에서 설정한 커스텀 오브젝트 타입 적용 및 오버랩 허용 (에디터 뷰포트용)
  if (SphereCollision) {
    SphereCollision->SetCollisionObjectType(ObjectType);
    SphereCollision->SetCollisionResponseToChannel(ObjectType, ECR_Overlap);
  }
  
  SetItemDataAndApply(ItemData);
  
  // 모든 로직 모듈에 에디터 미리보기 기회 제공
  if (HasAuthority() && ItemData) {
    if (InteractableComponent) {
      InteractableComponent->InitializeLogic(ItemData, this);
    }
    if (BlackboardComponent) {
      BlackboardComponent->MarkLogicInitialized(ItemData);
    }
  }
  for (ULogicModuleBase *Module : GetLogicModules()) {
    if (Module) {
      Module->OnConstructionLogic(this);
    }
  }
}