// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "MarkerData.generated.h"

class UNiagaraSystem;
class UUserWidget;
class AMarkerBase;

/**
 * MarkerBase 액터를 정의하는 데이터 에셋.
 * 로직 모듈, 시각 이펙트, UI 위젯, 감지 반경을 구성합니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UMarkerData : public ULogicEntityDataBase
{
    GENERATED_BODY()

public:
    /** 스폰할 MarkerBase 서브클래스 (nullptr이면 AMarkerBase 기본 클래스 사용) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker|Spawn")
    TSubclassOf<AMarkerBase> MarkerClass;

    /** 마커 위치를 표시할 Niagara 이펙트 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker|Visual")
    TObjectPtr<UNiagaraSystem> MarkerEffect;

    /** 상호작용 안내 UI 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker|Visual")
    TSubclassOf<UUserWidget> MarkerWidgetClass;

    /** InteractorComponent의 DetectionSphere가 감지할 반경 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker|Detection",
              meta = (ClampMin = "50.0"))
    float DetectionRadius = 200.f;
};
