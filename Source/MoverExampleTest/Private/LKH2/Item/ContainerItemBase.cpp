#include "LKH2/Item/ContainerItemBase.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Component/CarryInteractComponent.h"
#include "LKH2/Logic/LogicModuleBase.h"

AContainerItemBase::AContainerItemBase() {
  InteractComponent = CreateDefaultSubobject<UCarryInteractComponent>(
      TEXT("InteractComponent"));
  InteractComponent->SetupAttachment(RootComponent);
}

void AContainerItemBase::BeginPlay() {
  Super::BeginPlay();

  // Logic initialization is now handled by ItemBase::BeginPlay via LogicContextComponent
}

void AContainerItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);
}

bool AContainerItemBase::OnCarryInteract_Implementation(const FCarryContext &Context) {
  
  // 1. 던지기(Throw)는 아이템 본체 기능(물리적 이동)을 최우선합니다.
  if (Context.InteractionType == ECarryInteractionType::Throw) {
    return Super::OnCarryInteract_Implementation(Context);
  }

  // 2. 그 외 상호작용(Interact 등)은 워크스테이션(담기/조합) 로직 시도
  if (InteractComponent) {
    if (InteractComponent->OnInteract(Context)) {
      return true;
    }
  }

  // 3. 폴백: 본체 기능 (줍기 등)
  return Super::OnCarryInteract_Implementation(Context);
}

UCarryInteractComponent *AContainerItemBase::GetCarryInteractComponent() const {
  return InteractComponent;
}
