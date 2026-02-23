// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCommonMacros.h"
#include "Blueprint/UserWidget.h"
#include "ServerSelectWidget.generated.h"

DECLARE_DYNAMIC_DELEGATE(FButtonUpdateDelegate);

class UOnlineGameInstanceSubsystem;
class UServerSearchRowWidget;
/**
 *
 */
UCLASS()
class MOVEREXAMPLETEST_API UServerSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UServerSelectWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	void RoomSelected(UServerSearchRowWidget* Widget, int32 Index);

private:

	UFUNCTION()
	void OpenMainMenu();
	UFUNCTION()
	void OpenJoinMenu();
	UFUNCTION()
	void OpenHostMenu();
	UFUNCTION()
	void HostServer();
	UFUNCTION()
	void RefreshServer();
	UFUNCTION()
	void JoinServer();
	UFUNCTION()
	void OnPlayerCountSet(const FText& Text);

	UFUNCTION()
	void OnServerRefreshed(bool bWasSuccessful);

private:

#pragma region BindWidget
	// BindWidgets
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"), BlueprintReadOnly)
	class UWidgetSwitcher* MenuSwitcher;

	// WIP BUTTON WILL HERE?


	// MainMenu
	UPROPERTY(meta = (BindWidget, AllowprivateAccess = "true"), BlueprintReadOnly)
	class UWidget* MainMenuWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	// End Mainmenu

	// ServerMenu
	UPROPERTY(meta = (BindWidget))
	class UWidget* HostMenuWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelServerButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostServerButton;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* RoomNameEText;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* PlayerCountEText;


	// End ServerMenu

	// JoinMenu
	UPROPERTY(meta = (BindWidget))
	class UWidget* JoinMenuWidget;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* RoomList;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelJoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinServerButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UButton* RefreshServerButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* RefreshServerText;
	// End JoinMenu

#pragma endregion

private:


	TObjectPtr<UOnlineGameInstanceSubsystem> OnlineSubsystem;
	FDelegateHandle RefreshServerHandle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UServerSearchRowWidget> ServerSearchRowWidgetClass;

	int32 MaxPlayers = 8; // just for testing, this should be configured later!!!

	int32 SelectedRowIndex = 0;

	UServerSearchRowWidget* SelectedRow;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsRefreshing = false;

	UPROPERTY(BlueprintReadWrite, Meta = (AllowprivateAccess = true));
	FButtonUpdateDelegate UpdateServerButtonDelegate;

	FDelegateHandle OnServerRefreshedDelegate_Handle;

};
