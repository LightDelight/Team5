// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "CartPropertyComponent.generated.h"

class UBoxComponent;
class UCartData;
class ULogicModuleBase;
class UInteractablePropertyComponent;
class ULogicContextComponent;

/**
 * 카트(Cart) 액터에 부착하는 올인원 컴포넌트.
 *
 * InteractableComponent의 기능을 내장하여 별도의 InteractableComponent 없이
 * CartData의 모듈을 직접 초기화·실행합니다.
 *
 * 역할:
 *  - 생성자에서 UBoxComponent를 자식으로 생성 → 에디터에서 크기/위치 확인 가능
 *  - BoxCollision Overlap 시 'Intent.Cart.ItemOverlap' Context를 구성해 스스로에게 전송
 *  - Tick에서 전복 각도를 감지하면 'Intent.Cart.Overturn' Context를 구성해 스스로에게 전송
 *  - OnInteract() 호출 시 CartData의 로직 모듈을 순회하여 처리
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UCartPropertyComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UCartPropertyComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void OnRegister() override;

public:
    // ─── CartData ───

    /** 카트 데이터 에셋. Logic_Cart_Snap, Logic_Cart_Spill 등 모듈을 등록합니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart")
    TObjectPtr<UCartData> CartData;

    // ─── BoxCollision (생성자에서 CreateDefaultSubobject로 생성 → 에디터에서 확인 가능) ───

    /** 아이템 감지용 Box Collision (에디터에서 크기·위치 직접 조정 가능) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cart|Components")
    TObjectPtr<UBoxComponent> DetectionBox;

    // ─── 전복 감지 설정 ───

    /**
     * 카트 UpVector와 세계 Z축 사이 각도(도)가 이 값을 초과하면 전복으로 판정합니다.
     * 기본값 70도: 카트가 심하게 기울었을 때만 반응.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart|Overturn",
              meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float OverturnAngleThreshold = 70.0f;

    /** 전복 판정 이후 중복 발동을 막기 위한 쿨다운(초). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart|Overturn",
              meta = (ClampMin = "0.0"))
    float OverturnCooldown = 3.0f;

    // ─── InteractableComponent 기능 내장 ───

    /**
     * CartData의 로직 모듈을 순회하여 상호작용을 처리합니다.
     * CartPropertyComponent 자기 자신이 메시지 발신자 겸 수신자 역할을 합니다.
     */
    bool OnInteract(const FInteractionContext& Context);

    /** 초기화된 로직 모듈 목록을 반환합니다. */
    TArray<ULogicModuleBase*> GetLogicModules() const { return LogicModules; }

    /** 로직 모듈 초기화 여부를 반환합니다. */
    bool IsLogicInitialized() const { return bLogicInitialized; }

private:
    /** Overlap 이벤트 핸들러 */
    UFUNCTION()
    void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
                           AActor* OtherActor,
                           UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex,
                           bool bFromSweep,
                           const FHitResult& SweepResult);

    /** CartData에서 로직 모듈을 인스턴스화합니다. BeginPlay에서 호출됩니다. */
    void InitializeLogic();

    /** 마지막으로 전복 Intent를 발송한 이후 경과 시간 (쿨다운용) */
    float TimeSinceLastOverturn = 0.0f;

    /** 이중 초기화 방지 플래그 */
    UPROPERTY(Transient)
    bool bLogicInitialized = false;

    /** CartData에서 인스턴스화된 로직 모듈 목록 */
    UPROPERTY(Transient)
    TArray<TObjectPtr<ULogicModuleBase>> LogicModules;
};
