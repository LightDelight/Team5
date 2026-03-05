// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "InteractableComponent.generated.h"

class ULogicEntityDataBase;
class ULogicModuleBase;


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UInteractableComponent : public USceneComponent {
  GENERATED_BODY()

public:
  UInteractableComponent();

  // 부모 액터(인터페이스)로부터 전달받은 메시지 처리
  bool OnInteract(const FInteractionContext &Context);
  void SetOutlineEnabled(bool bEnabled);

  /**
   * 데이터 에셋 기반으로 로직 모듈들을 초기화합니다.
   * 이미 초기화된 경우 무시됩니다.
   */
  void InitializeLogic(ULogicEntityDataBase* InData, AActor* Context);

  /** 보관 중인 로직 모듈 목록을 반환합니다. */
  TArray<ULogicModuleBase *> GetLogicModules() const { return LogicModules; }

  /** 초기화 여부를 반환합니다. */
  bool IsLogicInitialized() const { return bLogicInitialized; }

protected:

protected:
  virtual void BeginPlay() override;

private:
  // 경쟁 상태(Race Condition) 관리: 이미 누군가 상호작용 중인지 확인
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<AActor> CurrentInteractor;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction",
            meta = (AllowPrivateAccess = "true"))
  bool bIsInteracting;

  /** 이중 초기화 방지용 플래그 */
  UPROPERTY(Transient)
  bool bLogicInitialized = false;

  /** 데이터 에셋 참조를 캐싱합니다 */
  UPROPERTY(Transient)
  TObjectPtr<ULogicEntityDataBase> EntityData;

  /** 이 엔티티의 로직 모듈 인스턴스/CDO 목록 */
  UPROPERTY(Transient)
  TArray<TObjectPtr<ULogicModuleBase>> LogicModules;
};
