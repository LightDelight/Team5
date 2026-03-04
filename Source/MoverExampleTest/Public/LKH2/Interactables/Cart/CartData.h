// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "CartData.generated.h"

/**
 * 카트(Cart) 엔티티의 데이터 에셋.
 *
 * 현재는 부모 클래스(LogicEntityDataBase)의 AdditionalModules와 EntityStats를 그대로 활용합니다.
 * 차후 카트 전용 비주얼/설정 필드(스폰 클래스, 메쉬 등)를 추가할 수 있습니다.
 */
UCLASS(BlueprintType)
class MOVEREXAMPLETEST_API UCartData : public ULogicEntityDataBase
{
    GENERATED_BODY()

    // AdditionalModules (Logic_Cart_Snap, Logic_Cart_Spill 등을 꽂아 사용)
    // EntityStats (차후 카트 관련 수치 기록용으로 확장 가능)
};
