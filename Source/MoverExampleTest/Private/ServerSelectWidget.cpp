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


UServerSelectWidget::UServerSelectWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//ConstructorHelpers::FClassFinder<UUserWidget> ServerRowWidgetClassAsset = (TEXT(""));
	//if (ServerRowWidgetClassAsset.Succeeded())
}

void UServerSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	check(ServerSearchRowWidgetClass);

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
		OnlineSubsystem->CreateSession(*RoomNameEText->GetText().ToString(), Players);
		return;
	}
	PlayerCountEText->SetText(FText::FromString(FString::Printf(TEXT("OnlineSubsystem failed"))));
}

void UServerSelectWidget::RefreshServer()
{
	if (IsRefreshing == true) return;
	IsRefreshing = true;
	check(UpdateServerButtonDelegate.IsBound());
	UpdateServerButtonDelegate.Execute();
	RefreshServerText->SetText(FText::FromString("Refreshing Server..."));
	SelectedRowIndex = -1;
	RoomList->ClearChildren();
	check(!OnServerRefreshedDelegate_Handle.IsValid());
	check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
	OnServerRefreshedDelegate_Handle = OnlineSubsystem->GetSessionInterface()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnServerRefreshed);
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

void UServerSelectWidget::OnServerRefreshed(bool bWasSuccessful)
{

	IsRefreshing = false;
	check(UpdateServerButtonDelegate.IsBound());
	UpdateServerButtonDelegate.Execute();
	RefreshServerText->SetText(FText::FromString("Refresh Server"));
	check(OnServerRefreshedDelegate_Handle.IsValid());
	check(OnlineSubsystem && OnlineSubsystem->GetSessionInterface());
	OnlineSubsystem->GetSessionInterface()->ClearOnFindSessionsCompleteDelegate_Handle(OnServerRefreshedDelegate_Handle);

#if UE_BUILD_DEVELOPMENT
	OnServerRefreshedDelegate_Handle.Reset();
#endif

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
