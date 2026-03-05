// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerSelectWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ServerSearchRowWidget.h"
#include "OnlineGameInstanceSubsystem.h"
#include "MyCommonMacros.h"

UServerSelectWidget::UServerSelectWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//ConstructorHelpers::FClassFinder<UUserWidget> ServerRowWidgetClassAsset = (TEXT(""));
	//if (ServerRowWidgetClassAsset.Succeeded())
}

void UServerSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	check(ServerSearchRowWidgetClass);
	check(!LevelToCreate.IsNull());

	if (GetGameInstance()) { // editor null access crash prevention
		OnlineSubsystem = GetGameInstance()->GetSubsystem<UOnlineGameInstanceSubsystem>();
		check(OnlineSubsystem);


	}

	JoinButton->OnClicked.AddDynamic(this, &UServerSelectWidget::OpenJoinMenu);
	HostButton->OnClicked.AddDynamic(this, &UServerSelectWidget::OpenHostMenu);
	CancelServerButton->OnClicked.AddDynamic(this, &UServerSelectWidget::OpenMainMenu);
	HostServerButton->OnClicked.AddDynamic(this, &UServerSelectWidget::HostServer);
	CancelJoinButton->OnClicked.AddDynamic(this, &UServerSelectWidget::OpenMainMenu);
	JoinServerButton->OnClicked.AddDynamic(this, &UServerSelectWidget::JoinServer);
	RefreshServerButton->OnClicked.AddDynamic(this, &UServerSelectWidget::RefreshServer);
	PlayerCountEText->OnTextChanged.AddDynamic(this, &UServerSelectWidget::OnPlayerCountSet);

}

void UServerSelectWidget::NativeConstruct()
{
	FInputModeUIOnly Inputmode;
	UWorld* World = GetWorld();
	check(World);

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC) {
		PC->SetInputMode(Inputmode);
		PC->bShowMouseCursor = true;
	}
}

void UServerSelectWidget::NativeDestruct()
{
	if (OnlineSubsystem) {
		//invalidate handle if exist;
	}

	FInputModeGameOnly InputMode;

	UWorld* World = GetWorld();
	check(World);
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC) {
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
	Super::NativeDestruct();
}

void UServerSelectWidget::RoomSelected(UServerSearchRowWidget* Widget, int32 Index)
{
	if (SelectedRow) {
		SelectedRow->DeselectRow();
	}
	SelectedRowIndex = Index;
	SelectedRow = Widget;
}

void UServerSelectWidget::OpenMainMenu()
{
	if (bActionBlockFence == true) { return; }
	MenuSwitcher->SetActiveWidget(MainMenuWidget);

}

void UServerSelectWidget::OpenJoinMenu()
{
	MenuSwitcher->SetActiveWidget(JoinMenuWidget);
	RefreshServer();

}

void UServerSelectWidget::OpenHostMenu()
{
	MenuSwitcher->SetActiveWidget(HostMenuWidget);
}

void UServerSelectWidget::HostServer()
{
	int32 Players = FCString::Atoi(*PlayerCountEText->GetText().ToString());
	if (Players > MaxPlayers) {
		PlayerCountEText->SetText(FText::FromString(FString::Printf(TEXT("Maximum player number is %d, Please enter lower"), MaxPlayers)));
		return;
	}
	if (Players < 2) {
		PlayerCountEText->SetText(FText::FromString(FString::Printf(TEXT("At least 2 player required"))));
		return;
	}
	if (OnlineSubsystem) {
		if (bActionBlockFence == true) { return; }
		bActionBlockFence = true;
		PRINT(TEXT("blockfence %hs"),bActionBlockFence?"True": "False");
		check(!OnSessionCreatedDelegate_Handle.IsValid());
		check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
		OnSessionCreatedDelegate_Handle = OnlineSubsystem->GetSessionInterface()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnSessionCreated);
		OnlineSubsystem->CreateSession(*RoomNameEText->GetText().ToString(), Players);
		return;
	}
	PlayerCountEText->SetText(FText::FromString(FString::Printf(TEXT("OnlineSubsystem failed"))));
}

void UServerSelectWidget::RefreshServer()
{
	if (bActionBlockFence == true) return;
	bActionBlockFence = true;
	check(UpdateServerButtonDelegate.IsBound());
	UpdateServerButtonDelegate.Execute();
	RefreshServerText->SetText(FText::FromString("Refreshing Server..."));
	SelectedRowIndex = -1;
	RoomList->ClearChildren();
	check(!OnSessionRefreshedDelegate_Handle.IsValid());
	check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
	OnSessionRefreshedDelegate_Handle = OnlineSubsystem->GetSessionInterface()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnSessionRefreshed);
	OnlineSubsystem->FindSessions();
}

void UServerSelectWidget::JoinServer()
{
	if (SelectedRowIndex < 0)
		return;
	OnlineSubsystem->JoinSession(SelectedRowIndex);
	// no player blocking. gamemode's responsiblity
}

void UServerSelectWidget::OnPlayerCountSet(const FText& Text)
{
	FString Sanitized = Text.ToString();
	if (Sanitized.Len() > 0) {
		if (!Sanitized.IsNumeric())
			Sanitized.RemoveAt(Sanitized.Len() - 1);
		if (!Sanitized.IsNumeric())
			Sanitized.Reset();
		PlayerCountEText->SetText(FText::FromString(Sanitized));
	}
}

void UServerSelectWidget::OnSessionCreated(FName SessionName, bool bWasSuccessful)
{
	check(OnSessionCreatedDelegate_Handle.IsValid());
	check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
	if (!bWasSuccessful) { check(bWasSuccessful); bActionBlockFence = false; return; }
	OnlineSubsystem->GetSessionInterface()->ClearOnCreateSessionCompleteDelegate_Handle(OnSessionCreatedDelegate_Handle);

	check(GetWorld() != nullptr);
	if (!GetWorld()) return;
	GetWorld()->ServerTravel(LevelToCreate.GetLongPackageName() + FString{TEXT("?listen")});
}

void UServerSelectWidget::OnSessionRefreshed(bool bWasSuccessful)	
{

	bActionBlockFence = false;
	check(UpdateServerButtonDelegate.IsBound());
	UpdateServerButtonDelegate.Execute();
	RefreshServerText->SetText(FText::FromString("Refresh Server"));
	check(OnSessionRefreshedDelegate_Handle.IsValid());
	check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
	OnlineSubsystem->GetSessionInterface()->ClearOnFindSessionsCompleteDelegate_Handle(OnSessionRefreshedDelegate_Handle);

	if (!bWasSuccessful) {
		RoomList->ClearChildren();
		return;
	}
	const FOnlineSessionSearch* SessionSearch = OnlineSubsystem->GetSessionSearch();

	int i = 0;
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults) {
		FString Room = "";
		UServerSearchRowWidget* ServerRow = CreateWidget<UServerSearchRowWidget>(GetWorld(), ServerSearchRowWidgetClass);
		check(ServerRow);
		SearchResult.Session.SessionSettings.Get(KEY_ROOM_NAME, Room);
		int Players = SearchResult.Session.NumOpenPublicConnections;
		int PlayerLimit = SearchResult.Session.SessionSettings.NumPublicConnections;
		ServerRow->SetTexts(SearchResult);
		ServerRow->SetIndex(i++);
		ServerRow->SetOwner(this);
		RoomList->AddChild(ServerRow);
	}
}
