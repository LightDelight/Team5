#include "SpillManager.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"

ASpillManager::ASpillManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerZone"));
    RootComponent = TriggerZone;
    TriggerZone->SetSphereRadius(300.0f);
    TriggerZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASpillManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASpillManager, CleanProgress);
}

void ASpillManager::RegisterItems(const TArray<AItemBase*>& Items)
{
    SpilledItems = Items;
    // 아이템들을 Spilled 상태로 만들고 위치 이동시키는 로직은 여기서 호출하거나
    // 카트가 호출
}

void ASpillManager::Server_ProcessInteraction_Implementation(float DeltaTime)
{
    CleanProgress += (DeltaTime / RequiredTime);
    CleanProgress = FMath::Clamp(CleanProgress, 0.0f, 1.0f);

    if (CleanProgress >= 1.0f)
    {
        // 완료! 아이템 정리
        for (AItemBase* Item : SpilledItems)
        {
            if (Item)
            {
                // InCart 상태로 전환하거나 Destroy
                // Item->Server_SetInCart(...)
                Item->Destroy(); // 예시
            }
        }
        Destroy(); // 매니저 삭제
    }
}

void ASpillManager::Server_StopInteraction_Implementation()
{
    // 천천히 감소
    CleanProgress = FMath::Max(0.0f, CleanProgress - GetWorld()->GetDeltaSeconds());
}

void ASpillManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // [Client Visual] 빨려 들어가는 연출
    if (CleanProgress > 0.0f && SpilledItems.Num() > 0)
    {
        FVector Center = GetActorLocation() + FVector(0, 0, 150.0f); // 공중으로 모임

        for (AItemBase* Item : SpilledItems)
        {
            if (IsValid(Item))
            {
                // ItemBase의 VisualMesh에 접근하여 위치 조작
                // 주의: ItemBase가 Spilled 상태여야 VisualMesh가 독립적으로 움직임

                UStaticMeshComponent* Mesh = Item->FindComponentByClass<UStaticMeshComponent>();
                if (Mesh)
                {
                    // 원래 위치(Item Actor Location)와 타겟 위치 보간
                    FVector StartPos = Item->GetActorLocation();
                    FVector TargetPos = Center;

                    // Progress에 따라 Lerp + Shake
                    FVector LerpedPos = FMath::Lerp(StartPos, TargetPos, CleanProgress * 0.8f);
                    FVector Shake = FMath::VRand() * (CleanProgress * 20.0f);

                    // Mesh 위치 강제 설정 (Physics 시뮬레이션 중이라도 덮어쓰기)
                    Mesh->SetWorldLocation(LerpedPos + Shake);
                }
            }
        }
    }
}