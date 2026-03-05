// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "MarkerBase.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UMarkerData;
class UInteractableComponent;
class UInteractablePropertyComponent;
class ULogicContextComponent;
struct FItemStatValue;

/**
 * 고정형 경량 상호작용 액터. GridManager 등록 없이 Sphere Overlap으로 감지.
 *
 * - 물리/이동/ItemStateComponent 없음 → 완전 고정
 * - MarkerData DA로 Niagara 이펙트, UI 위젯, 모듈 구성
 * - InteractablePropertyComponent로 결합도 높은 데이터(위젯, StoredItems 등) 보관
 */
UCLASS()
class MOVEREXAMPLETEST_API AMarkerBase : public AActor,
                                         public IInteractionContextInterface,
                                         public ILogicContextInterface
{
    GENERATED_BODY()

public:
    AMarkerBase();

    /** 데이터 에셋을 설정하고 외형/상호작용 설정을 즉시 적용합니다. */
    UFUNCTION(BlueprintCallable, Category = "Marker|Data")
    void SetMarkerDataAndApply(UMarkerData* InData);

    // ─── ILogicContextInterface 구현 ───
    virtual UInteractableComponent* GetInteractableComponent() const override;
    virtual UInteractablePropertyComponent* GetPropertyComponent() const override;
    virtual const FItemStatValue* FindStat(const FGameplayTag& Tag) const override;
    virtual void SetStat(const FGameplayTag& Tag, const FItemStatValue& Value) override;
    virtual FGameplayTag ResolveKey(const FGameplayTag& Key) const override;
    virtual TArray<ULogicModuleBase*> GetLogicModules() const override;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ─── IInteractionContextInterface 구현 ───
    virtual bool OnInteract_Implementation(const FInteractionContext& Context) override;
    virtual void SetOutlineEnabled_Implementation(bool bEnabled) override;

protected:
    /** 감지용 Sphere Collision (InteractorComponent의 DetectionSphere가 Overlap 감지) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker|Components")
    TObjectPtr<USphereComponent> DetectionSphere;

    /** 위치 표시 Niagara 이펙트 (DA에서 설정) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker|Components")
    TObjectPtr<UNiagaraComponent> EffectComponent;

    /** 로직 모듈 파이프라인 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker|Components")
    TObjectPtr<UInteractableComponent> InteractableComponent;

    /**
     * 결합도 높은 데이터 보관 (위젯, StoredItems 등).
     * 내장 ProgressWidgetComponent로 UI 안내도 처리.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker|Components")
    TObjectPtr<UInteractablePropertyComponent> PropertyComponent;

    /** 런타임 블랙보드 (로직 모듈 간 데이터 공유) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker|Components")
    TObjectPtr<ULogicContextComponent> BlackboardComponent;

    /** 마커를 정의하는 데이터 에셋 */
    UPROPERTY(ReplicatedUsing = OnRep_MarkerData, EditAnywhere,
              BlueprintReadOnly, Category = "Marker|Data")
    TObjectPtr<UMarkerData> MarkerData;

    UFUNCTION()
    void OnRep_MarkerData();

private:
    void ApplyMarkerData();

    /** ApplyMarkerData 중복 호출 방지 플래그 */
    bool bDataApplied = false;
};
