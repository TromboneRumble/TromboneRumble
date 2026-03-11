// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "CommonActivatableWidget.h"
#include "EasyMatchmakingManager.h"
#include "EasyPartyManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMainUIRoot::PushMenu(const EMainMenuType InType) const
{
	TSubclassOf<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	switch (InType)
	{
		case EMainMenuType::MainMenu:
			TargetWidgetClass = DefaultWidgetClass;
			break;
		case EMainMenuType::Lobby:
			TargetWidgetClass = LobbyWidgetClass;
			break;
		case EMainMenuType::Settings:
			TargetWidgetClass = SettingMenuWidgetClass;
			break;
		default:
			break;
	}
	
	if (TargetWidgetClass)
	{
		UIStack->AddWidget(TargetWidgetClass);
	}
}

void UMainUIRoot::Register()
{
	Super::Register();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingCompleted);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainUIRoot::HandleMatchmakingStarted()
{
	PushLoadingOverlay();
}

void UMainUIRoot::HandleMatchmakingCompleted(FName SessionName, EEasyMatchmakingCompleteResult Result)
{
	PopLoadingOverlay();
}

void UMainUIRoot::HandleMatchmakingCanceled()
{
	PopLoadingOverlay();
}
