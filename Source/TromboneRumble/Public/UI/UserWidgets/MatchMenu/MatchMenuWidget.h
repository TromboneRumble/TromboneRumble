// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

enum class ERotatorDirection : uint8;
class UCommonRotatorWidgetBase;
enum class EMatchType : uint8;
class UCommonRotator;
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
	UFUNCTION()
	void OnMatchTypeChanged(EMatchType NewType);
	// ~ End GameState Events
	
	UFUNCTION()
	void HandleStartButtonClicked();
	UFUNCTION()
	void HandleBackButtonClicked();
	UFUNCTION()
	void HandleInviteButtonClicked();
	UFUNCTION()
	void HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir);

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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonRotatorWidgetBase> CR_MatchType;
	// ~ End UIs
	
	UPROPERTY(Transient)
	FString CachedMainMenuMapPath = "";
	UPROPERTY(Transient)
	FString CachedLobbyMapPath = "";
	
	bool bIsStarted = false;
};