// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LogicModuleBase.generated.h"

/**
 * 기본 데이터를 가지고 있거나, 로직을 담을 수 있는 빈 껍데기 모듈
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class MOVEREXAMPLETEST_API ULogicModuleBase : public UObject {
  GENERATED_BODY()
};
