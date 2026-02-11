// Fill out your copyright notice in the Description page of Project Settings.

#include "OnlineGameInstanceSubsystem.h"

static const FName KEY_ROOM_NAME{ TEXT("KEY_ROOM_NAME") };


bool UOnlineGameInstanceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{

#if UE_SERVER
	return false;
#else
	return Super::ShouldCreateSubsystem(Outer);
#endif
}

void UOnlineGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	LOG_FUNCTION_NAME;
	Super::Initialize(Collection);
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	check(OnlineSubsystem != nullptr);
	SessionInterface = OnlineSubsystem->GetSessionInterface();
	check(SessionInterface != nullptr);

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnCreateSessionComplete);
	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnDestorySessionComplete);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnFindSessionComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnJoinSessionComplete);
	SessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnUpdateSessionComplete);

	//SessionInterface->OnSessionParticipantJoinedDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnSessionParticipantJoined);
	//SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnSessionParticipantLeft);

	GEngine->OnNetworkFailure().AddUObject(this, &UOnlineGameInstanceSubsystem::OnNetworkFailure);
}

void UOnlineGameInstanceSubsystem::Deinitialize()
{
	LOG_FUNCTION_NAME;
	SessionInterface->OnCreateSessionCompleteDelegates.Clear();
	SessionInterface->OnDestroySessionCompleteDelegates.Clear();
	SessionInterface->OnFindSessionsCompleteDelegates.Clear();
	SessionInterface->OnJoinSessionCompleteDelegates.Clear();
	SessionInterface->OnUpdateSessionCompleteDelegates.Clear();

	Super::Deinitialize();
}

bool UOnlineGameInstanceSubsystem::CreateSession(const FString& RoomName, int32 AllowedPlayers)
{
	LOG_FUNCTION_NAME;
	if (SessionInterface->GetNamedSession(NAME_GameSession)) {
		LOG_TEMP(TEXT("Session already exists, might potential bug check flow"));
		DestroySession(FName(NAME_GameSession));
		check(false);
		return false;
	}
	else {
		FOnlineSessionSettings SessionSettings;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL") {
			SessionSettings.bIsLANMatch = true;
			SessionSettings.NumPublicConnections = AllowedPlayers;
			SessionSettings.bUsesPresence = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.Set(KEY_ROOM_NAME, RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
			return true;
		}
		else {
			LOG_TEMP(TEXT("Subsystem name is not null, configure required."))
		}
	}

	return false;
	//some fuctions does not work with null subsystem. i can't find any documentation about...
}

bool UOnlineGameInstanceSubsystem::DestroySession(FName SessionName) const
{
	LOG_FUNCTION_NAME;
	if (SessionInterface->GetNamedSession(SessionName)) {
		SessionInterface->DestroySession(SessionName);
		GetGameInstance()->ReturnToMainMenu();
		return true;
	}
	return false;
}

bool UOnlineGameInstanceSubsystem::FindSessions()
{
	LOG_FUNCTION_NAME;
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	check(SessionSearch != nullptr);
	SessionSearch->MaxSearchResults = 3; // magic number for testing
	SessionSearch->QuerySettings.Set(FName(TEXT("IT IS NOT WORK WITH NULL SUBSYS")), true, EOnlineComparisonOp::Equals);
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	return false;
}

bool UOnlineGameInstanceSubsystem::JoinSession(uint32 Index) const
{
	check(SessionInterface.IsValid());
	check(SessionSearch.IsValid());
	if (SessionSearch->SearchResults.Num() > static_cast<int32>(Index)) {
		if (SessionInterface->GetNamedSession(NAME_GameSession)) {
			SessionInterface->DestroySession(NAME_GameSession);
			LOG_TEMP(TEXT("Session already exists, might potential bug check flow"));
		}
		SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[Index]);
		return true;
	}
	return false;
}

void UOnlineGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	LOG_FUNCTION_NAME;
	check(GetWorld() != nullptr);
	GetWorld()->ServerTravel("/Game/ThirdPerson/PhysicsActorTest/PhysicsActorTest?listen");// hardcoded map name here edit as your will
}

void UOnlineGameInstanceSubsystem::OnDestorySessionComplete(FName SessionName, bool bWasSuccessful)
{
	LOG_FUNCTION_NAME;
}

void UOnlineGameInstanceSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	LOG_FUNCTION_NAME;
	check(bWasSuccessful && SessionSearch.IsValid());
	int i = 0;
	FString RoomName = "";
	LOG_TEMP(TEXT("FOUND %d Sessions"), SessionSearch->SearchResults.Num());
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults) {
		//fallback for ossnull
		SearchResult.Session.SessionSettings.Get(KEY_ROOM_NAME, RoomName);
		LOG_TEMP(TEXT("%d: <%s> %d/%d %3dms %s"),
			i++,
			*RoomName,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections,
			SearchResult.PingInMs,
			*SearchResult.Session.OwningUserId->ToString()
		);
		MY_PRINT(FString::Printf(TEXT("%d: <%s> %d/%d %3dms %s"),
			i - 1,
			*RoomName,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections,
			SearchResult.PingInMs,
			*SearchResult.Session.OwningUserId->ToString()
		)
		);
	}
	/**
	 * OnlineSubsystemNull NumOpenPublicConnections working ...
	 * it seems stop advertising session if NumOpenPublicConnections is 0
	 * but does not stop overconnecting when flooding.
	 *
	 * @TODO
	 * low priority feature for now.
	 * stop overconnectiong in AGameSession Server.
	 */

}

void UOnlineGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	LOG_FUNCTION_NAME;
	check(SessionInterface.IsValid());
	SessionSearch.Reset();

	FString Address;

	if (!SessionInterface->GetResolvedConnectString(SessionName, Address)) {
		LOG_TEMP(TEXT("ResolveConnectStringFail"));
	}
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (PC) {
		PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		LOG_TEMP(TEXT("JoinComplete"));
	}


}

void UOnlineGameInstanceSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccesful)
{
	LOG_FUNCTION_NAME;


}

void UOnlineGameInstanceSubsystem::OnSessionParticipantJoined(FName SessionName, const FUniqueNetId& NetId)
{
	LOG_FUNCTION_NAME;

}

void UOnlineGameInstanceSubsystem::OnSessionParticipantLeft(FName SEssionName, const FUniqueNetId& NetId, EOnSessionParticipantLeftReason Reason)
{
	LOG_FUNCTION_NAME;

}

void UOnlineGameInstanceSubsystem::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	LOG_FUNCTION_NAME;
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		DestroySession(FName(NAME_GameSession)); //hardcoded session name just for testing. should modified.
	}


}

//void UMainOnlineSubsystem::UpdatePlayerNum(int32 PlayerNum)
//{
//	CurrentPlayers = PlayerNum;
//	UE_LOG(LogGameInstanceSubsystemOnline, Log, TEXT("UpdateSessionInfoPlayer %d"), CurrentPlayers);
//	FOnlineSessionSettings* Settings;
//	if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession)) {
//		if (SessionInterface->GetNamedSession(NAME_GameSession)) {
//			Settings = SessionInterface->GetSessionSettings(NAME_GameSession);
//			Settings->Set(PLAYER_COUNT_KEY, CurrentPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
//			SessionInterface->UpdateSession(NAME_GameSession, *Settings);
//			UE_LOG(LogGameInstanceSubsystemOnline, Log, TEXT("UpdateSessionDone"), CurrentPlayers);
//			return;
//		}
//	}
//	UE_LOG(LogGameInstanceSubsystemOnline, Log, TEXT("%s Setting fail"), *FString(__func__));
//}
