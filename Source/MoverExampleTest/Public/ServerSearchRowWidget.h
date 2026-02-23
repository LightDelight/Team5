// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerSearchRowWidget.generated.h"

class UServerSelectWidget;
class FOnlineSessionSearchResult;

DECLARE_DYNAMIC_DELEGATE(FButtonClicked);

/**
 *
 */
UCLASS()
class MOVEREXAMPLETEST_API UServerSearchRowWidget : public UUserWidget
{
	GENERATED_BODY()


public:

	virtual void NativeOnInitialized() override;

	void SetOwner(UServerSelectWidget* Owner);

	void DeselectRow();
	void SetIndex(int32 Idx) { Index = Idx; }
	void SetTexts(const FOnlineSessionSearchResult& Result);

	int32 GetIndex() const { return Index; }

private:
	UFUNCTION()
	void SelectRow();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	class UButton* RowSelectButton;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RoomName;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerCount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SessionID;

public:


private:
	TSoftObjectPtr<UServerSelectWidget> Owner;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = "true"));
	int32 Index;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	bool Select{ false };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FButtonClicked UpdateSelection;

};

