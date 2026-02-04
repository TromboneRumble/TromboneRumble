// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "MatchMenuWidget.generated.h"

class UCommonTextBlock;
class UEasySessionSubsystem;
class UCommonButtonBase;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	
	virtual void Init() override;
	
private:
	// ~ Begin GameState Events
	void BindGameStateEvents();
	void RemoveGameStateEvents();
	
	UFUNCTION()
	void OnPlayerListChanged(const TArray<FString>& PlayerNames);
	// ~ End GameState Events
	
	UFUNCTION()
	void HandleStartButtonClicked();
	UFUNCTION()
	void HandleBackButtonClicked();
	UFUNCTION()
	void HandleInviteButtonClicked();

	virtual void SetUIEnabled(const bool bEnabled) override;
	
	// ~ Begin UIs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Start;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Back;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Invite;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Code;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_PlayerList;
	// ~ End UIs
	
	UPROPERTY(Transient)
	FString CachedMainMenuMapPath = "";
	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
	
	bool bIsStarted = false;
};