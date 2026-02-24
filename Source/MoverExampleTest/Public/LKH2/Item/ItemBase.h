// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LKH2/Carry/Interface/CarryInterface.h"
#include "LKH2/Logic/LogicContextInterface.h"
#include "LKH2/Logic/LogicInteractionInterface.h"
#include "ItemBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UItemData;
class UItemStateComponent;
class UItemSmoothingComponent;
class UCarryableComponent;
class ULogicContextComponent;

UCLASS()
class AItemBase : public AActor, public ICarryInterface, public ILogicContextInterface {
  GENERATED_BODY()

public:
  // 이 액터의 속성에 대한 기본값을 설정합니다
  AItemBase();

  class UItemData *GetItemData() const { return ItemData; }

  // ILogicContextInterface 구현
  virtual UCarryableComponent *GetCarryableComponent() const override;
  virtual UCarryInteractComponent *GetCarryInteractComponent() const override;
  virtual FLogicBlackboard *GetLogicBlackboard() override;
  virtual const FItemStatValue *FindStat(const FGameplayTag &Tag) const override;
  virtual void SetStat(const FGameplayTag &Tag,
                       const FItemStatValue &Value) override;
  virtual FGameplayTag ResolveKey(const FGameplayTag &Key) const override;
  virtual TArray<class ULogicModuleBase *> GetLogicModules() const override;

protected:
  // 게임이 시작되거나 스폰될 때 호출됩니다
  virtual void BeginPlay() override;
  virtual void OnConstruction(const FTransform &Transform) override;

protected:
  /** 충돌 및 물리 처리를 위한 루트 컴포넌트 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
  TObjectPtr<USphereComponent> SphereCollision;

  /** 시각적 메쉬 컴포넌트 (물리 없음, 루트에 부착되거나 보간됨) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
  TObjectPtr<UStaticMeshComponent> VisualMesh;

  /** 이 아이템을 정의하는 데이터 에셋 */
  UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ItemData, BlueprintReadOnly,
            Category = "Item|Data")
  TObjectPtr<UItemData> ItemData;

  UFUNCTION()
  void OnRep_ItemData();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
  TObjectPtr<UItemStateComponent> StateComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
  TObjectPtr<UItemSmoothingComponent> SmoothingComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Components")
  TObjectPtr<UCarryableComponent> CarryComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Logic")
  TObjectPtr<ULogicContextComponent> BlackboardComponent;

  // ICarryInterface
  virtual bool OnCarryInteract_Implementation(const FCarryContext &Context) override;
  virtual void SetOutlineEnabled_Implementation(bool bEnabled) override;

public:
  UFUNCTION(BlueprintCallable, Category = "Item|Data")
  void SetItemDataAndApply(UItemData *InData);

  UFUNCTION(BlueprintCallable, Category = "Item|Identifier")
  void SetInstanceId(const FGuid &InId) { InstanceId = InId; }

  UFUNCTION(BlueprintCallable, Category = "Item|Identifier")
  FGuid GetInstanceId() const { return InstanceId; }

protected:
  /** 매니저가 발급한 인스턴스 고유 식별자 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Item|Identifier")
  FGuid InstanceId;
};
