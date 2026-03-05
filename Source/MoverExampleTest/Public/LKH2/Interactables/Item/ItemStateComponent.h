// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "ItemStateComponent.generated.h"

UENUM(BlueprintType)
enum class EItemState : uint8 {
  Placed UMETA(DisplayName = "Placed"),
  Carried UMETA(DisplayName = "Carried"),
  Dropped UMETA(DisplayName = "Dropped"),
  Stored UMETA(DisplayName = "Stored"),
  Spilled UMETA(DisplayName = "Spilled") // 카트 전복 시 쏟아진 상태. 상호작용 불가, 로컬 물리 시뮬레이션.
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UItemStateComponent : public UActorComponent {
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UItemStateComponent();

protected:
  // Called when the game starts
  virtual void BeginPlay() override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

public:
  // Called every frame
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

  UPROPERTY(ReplicatedUsing = OnRep_ItemState, BlueprintReadOnly,
            Category = "Item State")
  EItemState CurrentState;

  UFUNCTION()
  void OnRep_ItemState();

  UFUNCTION(BlueprintCallable, Category = "Item State")
  void SetItemState(EItemState NewState);

  /**
   * 서버에서 호출하여 현재 부착 상태(부모 액터/컴포넌트)를 리플리케이션 변수에 갱신합니다.
   */
  UFUNCTION(BlueprintAuthorityOnly, Category = "Item State")
  void UpdateAttachmentReplication();

  /**
   * 아이템의 상태를 Dropped로 변경하고 임펄스를 가합니다.
   * 물리 및 충돌 활성화 처리는 SetItemState(Dropped) 내에서 처리됩니다.
   *
   * @param Impulse 가해질 던지기 힘
   */
  UFUNCTION(BlueprintCallable, Category = "Item State")
  void ThrowItem(const FVector &Impulse);

private:
  /** [Replication] 수동 동기화를 위한 부모 정보 */
  UPROPERTY(ReplicatedUsing = OnRep_AttachmentData)
  TObjectPtr<AActor> RepParentActor;

  UPROPERTY(ReplicatedUsing = OnRep_AttachmentData)
  TObjectPtr<USceneComponent> RepAttachComponent;

  UFUNCTION()
  void OnRep_AttachmentData();
};
