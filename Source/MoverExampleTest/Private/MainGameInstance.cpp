// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameInstance.h"

void UMainGameInstance::Init()
{
	LOG_FUNCTION_NAME;
	Super::Init();
	OnlineSubsystem = GetSubsystem<UOnlineGameInstanceSubsystem>();

}

void UMainGameInstance::Shutdown()
{
	LOG_FUNCTION_NAME;
	Super::Shutdown();
}
