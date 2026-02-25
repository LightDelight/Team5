#include "LKH2/Interactables/Item/ContainerItemBase.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"

AContainerItemBase::AContainerItemBase() {
  InteractComponent = CreateDefaultSubobject<UInteractableComponent>(
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

bool AContainerItemBase::OnInteract_Implementation(const FInteractionContext &Context) {
  
  // 1. 던지기(Throw)는 아이템 본체 기능(물리적 이동)을 최우선합니다.
  if (Context.InteractionType == EInteractionType::Throw) {
    return Super::OnInteract_Implementation(Context);
  }

  // 2. 그 외 상호작용(Interact 등)은 워크스테이션(담기/조합) 로직 시도
  if (InteractComponent) {
    if (InteractComponent->OnInteract(Context)) {
      return true;
    }
  }

  // 3. 폴백: 본체 기능 (줍기 등)
  return Super::OnInteract_Implementation(Context);
}
