// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MyCommonMacros.h"
#include "OnlineGameInstanceSubsystem.generated.h"



/**
 *  This GameInstanceSubsystem control session related jobs.
 */

UCLASS()
class MOVEREXAMPLETEST_API UOnlineGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/**
	 * @param	bool			true if succcess
	 * @param	RoomName		room name to create session
	 * @param	AllowedPlayers	maximum allowed players
	 */
	bool CreateSession(const FString& RoomName, int32 AllowedPlayers);
	bool DestroySession(FName SessionName) const;
	bool FindSessions();
	bool JoinSession(uint32 Index) const;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestorySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccesful);

	void OnSessionParticipantJoined(FName SessionName, const FUniqueNetId& NetId);
	void OnSessionParticipantLeft(FName SEssionName, const FUniqueNetId& NetId, EOnSessionParticipantLeftReason Reason);

	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString = TEXT(""));

private:
	IOnlineSessionPtr SessionInterface = nullptr;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
