// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "GameplayTagContainer.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"
#include "WorkstationData.generated.h"

class ULogicModuleBase;
class UPresetData;
class UVisualPresetData;
class UStaticMesh;
class AWorkStationBase;

/**
 * 워크스테이션(작업대) 데이터를 정의하는 클래스.
 * 투 트랙 구조:
 *   Track 1 — Preset 기반 (로직: PresetData, 비주얼: VisualPresetData)
 *   Track 2 — Custom (AdditionalModules, bUseCustomVisuals → Custom* 필드)
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UWorkstationData : public ULogicEntityDataBase {
  GENERATED_BODY()

public:
  // ─── 투 트랙: 로직 (부모 클래스에서 상속받음: EntityStats) ───

  // ─── 투 트랙: 비주얼 ───

  /** [Track 1] 비주얼 프리셋 (이름, 메쉬, 콜리전/오프셋 기본값) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workstation|Visual")
  TObjectPtr<UVisualPresetData> VisualPreset;

  /** [Track 2] 체크하면 아래 커스텀 비주얼 값을 사용, 해제하면 VisualPreset 사용
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Workstation|Visual")
  bool bUseCustomVisuals = false;

  /** [커스텀] 워크스테이션 스폰 클래스 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Workstation|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  TSubclassOf<AWorkStationBase> CustomWorkstationClass;

  /** [커스텀] 워크스테이션 메쉬 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Workstation|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  TObjectPtr<UStaticMesh> CustomWorkstationMesh;

  /** [커스텀] Box 콜리전 크기 (Half-Extent) */
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Workstation|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  FVector CustomBoxExtent = FVector(50.0f, 50.0f, 50.0f);

  /** [커스텀] Box 콜리전의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Workstation|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  FVector CustomBoxRelativeLocation = FVector::ZeroVector;

  /** [커스텀] CarryInteractComponent의 상대 위치 오프셋 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly,
            Category = "Workstation|Visual",
            meta = (EditCondition = "bUseCustomVisuals"))
  FVector CustomInteractRelativeLocation = FVector::ZeroVector;

  // ─── 헬퍼 ───

  /** 유효 스폰 클래스 반환 */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Visual")
  TSubclassOf<AWorkStationBase> GetEffectiveWorkstationClass() const;

  /** 유효 메쉬 반환 */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Visual")
  UStaticMesh *GetEffectiveWorkstationMesh() const;

  /** 유효 Box 콜리전 크기 반환 */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Visual")
  FVector GetEffectiveBoxExtent() const;

  /** 유효 Box 오프셋 반환 */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Visual")
  FVector GetEffectiveBoxRelativeLocation() const;

  /** 유효 InteractComponent 오프셋 반환 */
  UFUNCTION(BlueprintCallable, Category = "Workstation|Visual")
  FVector GetEffectiveInteractRelativeLocation() const;

};
