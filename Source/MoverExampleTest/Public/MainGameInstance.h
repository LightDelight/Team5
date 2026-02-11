// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyCommonMacros.h"
#include "OnlineGameInstanceSubsystem.h"
#include "MainGameInstance.generated.h"

/**
 *
 */
UCLASS()
class MOVEREXAMPLETEST_API UMainGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
	virtual void Shutdown() override;

protected:
	UFUNCTION(Exec)
	void CreateSession(FString Name = FString("DEFAULT NAME"), int32 Players = 3) { OnlineSubsystem->CreateSession(Name, Players); }
	UFUNCTION(Exec)
	void DestroySession() { OnlineSubsystem->DestroySession(FName(NAME_GameSession)); }
	UFUNCTION(Exec)
	void FindSessions() { OnlineSubsystem->FindSessions(); }
	UFUNCTION(Exec)
	void JoinSession(uint32 Index) { OnlineSubsystem->JoinSession(Index); }

private:
	TObjectPtr<UOnlineGameInstanceSubsystem> OnlineSubsystem = nullptr;
};