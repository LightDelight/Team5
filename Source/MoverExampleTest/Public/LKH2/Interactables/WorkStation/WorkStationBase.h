// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/Interaction/Base/LogicContextInterface.h"
#include "WorkStationBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UWorkstationData;
class UInteractableComponent;
class ULogicContextComponent;

UCLASS()
class MOVEREXAMPLETEST_API AWorkStationBase : public AActor,
                                              public IInteractionContextInterface,
                                              public ILogicContextInterface {
  GENERATED_BODY()

public:
  AWorkStationBase();

  /** 데이터 에셋을 설정하고 외형/상호작용 설정을 즉시 적용합니다. */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Data")
  void SetWorkstationDataAndApply(UWorkstationData *InData);

  // ILogicContextInterface 구현
  virtual UInteractableComponent *GetInteractableComponent() const override;
  virtual FLogicBlackboard *GetLogicBlackboard() override;
  virtual const FItemStatValue *FindStat(const FGameplayTag &Tag) const override;
  virtual void SetStat(const FGameplayTag &Tag,
                       const FItemStatValue &Value) override;
  virtual FGameplayTag ResolveKey(const FGameplayTag &Key) const override;
  virtual TArray<ULogicModuleBase *> GetLogicModules() const override;

protected:
  virtual void BeginPlay() override;
  virtual void OnConstruction(const FTransform &Transform) override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

protected:
  /** 고정형 루트로 사용할 메쉬 컴포넌트 (충돌 비활성화) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UStaticMeshComponent> RootMesh;

  /** 실질적인 충돌을 담당할 박스 콜리전 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UBoxComponent> BoxCollision;

  /** 이 워크스테이션을 정의하는 데이터 에셋 */
  UPROPERTY(ReplicatedUsing = OnRep_WorkstationData, EditAnywhere,
            BlueprintReadOnly, Category = "Workstation|Data")
  TObjectPtr<UWorkstationData> WorkstationData;

  /** 클라이언트에서 WorkstationData 리플리츀이트 시 메쉬 및 로직 모듈 초기화 */
  UFUNCTION()
  void OnRep_WorkstationData();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<UInteractableComponent> InteractableComponent;

  /** 런타임 상태 데이터를 통합 관리하는 컴포넌트 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Workstation|Components")
  TObjectPtr<ULogicContextComponent> BlackboardComponent;

  // IInteractionContextInterface 구현
  // 고정형이므로 들거나 던져지지는 않지만, 인터랙션(OnInteract) 및
  // 아웃라인(SetOutlineEnabled)로직에 사용
  virtual bool OnInteract_Implementation(const FInteractionContext &Context) override;
  virtual void SetOutlineEnabled_Implementation(bool bEnabled) override;

  // ── 그리드 스냅 (에디터 전용) ──

  /** 에디터에서 배치/이동 시 그리드 셀 중심에 자동 스냅 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workstation|Grid")
  bool bSnapToGrid = true;

  /** 스냅에 사용할 셀 크기 (GridManagerComponent의 CellSize와 동일하게 설정) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workstation|Grid",
            meta = (EditCondition = "bSnapToGrid", ClampMin = "10.0"))
  float SnapCellSize = 200.0f;

  /** 스냅에 사용할 그리드 원점 (GridManagerComponent의 GridOrigin과 동일하게 설정) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workstation|Grid",
            meta = (EditCondition = "bSnapToGrid"))
  FVector SnapGridOrigin = FVector::ZeroVector;
};
