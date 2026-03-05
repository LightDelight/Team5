// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerSearchRowWidget.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "ServerSelectWidget.h"
#include "Components/TextBlock.h"
#include "OnlineGameInstanceSubsystem.h"


void UServerSearchRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RowSelectButton->OnClicked.AddDynamic(this, &ThisClass::SelectRow);
	check(PlayerCount);

}

void UServerSearchRowWidget::SetOwner(UServerSelectWidget* O)
{
	Owner = O;
}

void UServerSearchRowWidget::SelectRow()
{
	check(Owner.IsValid());
	Owner->RoomSelected(this, Index);
	check(UpdateSelection.IsBound());
	Select = true;
	UpdateSelection.Execute();
}

void UServerSearchRowWidget::DeselectRow()
{
	check(UpdateSelection.IsBound());
	Select = false;
	UpdateSelection.Execute();

}

void UServerSearchRowWidget::SetTexts(const FOnlineSessionSearchResult& SearchResult)
{
	FString Room = "";
	SearchResult.Session.SessionSettings.Get(KEY_ROOM_NAME, Room);
	int Players = SearchResult.Session.NumOpenPublicConnections;
	int PlayerLimit = SearchResult.Session.SessionSettings.NumPublicConnections;
	PlayerCount->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), PlayerLimit - Players, PlayerLimit)));
	RoomName->SetText(FText::FromString(Room.Left(13)));
	SessionID->SetText(FText::FromString(SearchResult.Session.OwningUserName.Left(31)));
}
