#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

// 아이템 상태 정의
UENUM(BlueprintType)
enum class EItemState : uint8
{
    Basic       UMETA(DisplayName = "Basic"),      // 기본 (물리 밀림)
    Held        UMETA(DisplayName = "Held"),       // 들림 (캐릭터 손)
    Throwing    UMETA(DisplayName = "Throwing"),   // 던져짐 (수학적 궤적)
    Spilled     UMETA(DisplayName = "Spilled"),    // 쏟아짐 (로컬 물리 시뮬레이션)
    Cleaning    UMETA(DisplayName = "Cleaning"),   // 수습 중 (빨려들어감)
    InCart      UMETA(DisplayName = "In Cart")     // 카트 안 (관성 흔들림)
};

UCLASS()
class AItemBase : public AActor
{
    GENERATED_BODY()

public:
    AItemBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Components ---
    // 충돌 및 서버 위치 담당 (Logic Root)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCapsuleComponent* CapsuleComp;

    // 시각적 표현 담당 (Visual Mesh)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualMesh;

    // --- State Machine ---
    UPROPERTY(ReplicatedUsing = OnRep_CurrentState, BlueprintReadOnly, Category = "State")
    EItemState CurrentState;

    // 상태 변경 시 클라이언트에서 시각적 연출 처리를 위해 호출됨
    UFUNCTION()
    void OnRep_CurrentState();

    // --- Settings ---
    UPROPERTY(EditAnywhere, Category = "Settings")
    float ThrowHeight = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    bool bLockZAxis = true;

    // [New] 카트에 담겨있을 때 흔들림 강도 (0이면 고정)
    UPROPERTY(EditAnywhere, Category = "Settings|Visual")
    float InCartSwayStrength = 0.05f;

    // [New] 카트 흔들림 복원 속도
    UPROPERTY(EditAnywhere, Category = "Settings|Visual")
    float InCartSwaySpeed = 5.0f;

    // --- Throwing Logic Vars ---
    FVector ThrowStartPos;
    FVector ThrowTargetPos;
    float ThrowDuration;
    float ThrowStartTime;

    // --- Visual Interpolation Vars ---
    bool bVisualDetached = false; // Spilled 상태 등에서 메쉬가 루트를 벗어났는지 여부

public:
    // --- [Blueprint Callable API] ---

    // [Server] 아이템 던지기 시작
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Action")
    void Server_ThrowTo(FVector TargetPos, float Duration);

    // [Server] 쏟아짐 상태 전환
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Action")
    void Server_Spill(FVector LandPosition);

    // [Server] 줍기
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Action")
    void Server_PickUp(USceneComponent* ParentComp, FName SocketName);

    // [Internal] 비주얼 업데이트 로직
    void UpdateVisuals(float DeltaTime);
    void ApplyZLock();
};