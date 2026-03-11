// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "MainUIRoot.generated.h"

enum class EEasyMatchmakingCompleteResult : uint8;
class UCommonActivatableWidget;

enum class EMainMenuType : uint8
{
	None,
	MainMenu,
	Lobby,
	Settings
};

UCLASS()
class TROMBONERUMBLE_API UMainUIRoot : public UBaseUIRoot
{
	GENERATED_BODY()
	
public:
	void PushMenu(EMainMenuType InType) const;
	
protected:
	virtual void Register() override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> SettingMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> LobbyWidgetClass;
	
private:
	UFUNCTION()
	void HandleMatchmakingStarted();
	UFUNCTION()
	void HandleMatchmakingCompleted(FName SessionName, EEasyMatchmakingCompleteResult Result);
	UFUNCTION()
	void HandleMatchmakingCanceled();
};