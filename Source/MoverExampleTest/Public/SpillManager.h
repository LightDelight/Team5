#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.h"
#include "SpillManager.generated.h"

UCLASS()
class ASpillManager : public AActor
{
    GENERATED_BODY()

public:
    ASpillManager();

protected:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USphereComponent* TriggerZone; // 플레이어 감지용

    // --- Variables ---
    UPROPERTY(Replicated)
    float CleanProgress = 0.0f; // 0.0 ~ 1.0

    TArray<AItemBase*> SpilledItems; // 관리 대상 아이템들

    UPROPERTY(EditAnywhere)
    float RequiredTime = 3.0f;

public:
    // [Blueprint Callable] 쏟아진 아이템 등록
    UFUNCTION(BlueprintCallable)
    void RegisterItems(const TArray<AItemBase*>& Items);

    // [Blueprint Callable] 플레이어가 인터랙션 중일 때 호출 (Tick에서)
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_ProcessInteraction(float DeltaTime);

    // [Blueprint Callable] 인터랙션 중단
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_StopInteraction();
};