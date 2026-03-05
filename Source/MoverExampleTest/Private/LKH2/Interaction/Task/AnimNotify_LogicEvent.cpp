// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interaction/Task/AnimNotify_LogicEvent.h"
#include "LKH2/Interaction/Component/InteractorComponent.h"
#include "GameFramework/Pawn.h"

UAnimNotify_LogicEvent::UAnimNotify_LogicEvent()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 140, 0, 255); // 주황색 표시
#endif
}

void UAnimNotify_LogicEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 로컬 컨트롤 폰에서만 처리합니다.
	// 몽타주는 서버·클라이언트 양쪽에서 재생되므로, 이 체크 없이는
	// TryInteract()가 두 번 실행되어 진행도가 이중으로 카운트됩니다.
	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	UInteractorComponent* InteractorComp = OwnerPawn->FindComponentByClass<UInteractorComponent>();
	if (InteractorComp)
	{
		// TryInteract() 내부에서 Server_TryInteract RPC를 통해 서버로 전달됩니다.
		InteractorComp->TryInteract(EventTag);
	}
}
