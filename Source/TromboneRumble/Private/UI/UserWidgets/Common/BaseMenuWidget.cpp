// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "EasySessionSubsystem.h"
#include "EasyFriendSubsystem.h"
#include "GameFramework/HUD.h"
#include "UI/HUD/MainHUD.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"

void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BindSubsystemCallbacks();
	Init();
}

void UBaseMenuWidget::Init()
{
}

void UBaseMenuWidget::ShowNoticePopup(const FString& Content)
{
	if (NoticePopupWidgetClass)
	{
		// TODO : UI Stack에 넣어야 함
		UNoticePopupWidget* NoticePopup = CreateWidget<UNoticePopupWidget>(GetOwningPlayer(), NoticePopupWidgetClass);
		NoticePopup->OnInit(Content);
	}
}

void UBaseMenuWidget::BindSubsystemCallbacks()
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!SessionsSubsystem)
	{
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
	}
	if (!FriendsSubsystem)
	{
		FriendsSubsystem = GameInstance->GetSubsystem<UEasyFriendSubsystem>();
	}
	
	RemoveSubsystemCallbacks();
}

void UBaseMenuWidget::RemoveSubsystemCallbacks()
{
}

void UBaseMenuWidget::SetUIEnabled(const bool bEnabled)
{
}

void UBaseMenuWidget::ShowLoadingOverlay()
{
	if (const TObjectPtr<UBaseUIRoot> Root = GetRootLayout())
	{
		Root->PushLoadingOverlay();
	}
}

void UBaseMenuWidget::ShowLoadingOverlay(const FString InContent)
{
	if (const TObjectPtr<UBaseUIRoot> Root = GetRootLayout())
	{
		Root->PushLoadingOverlay(InContent);
	}
}

void UBaseMenuWidget::HideLoadingOverlay()
{
	if (const TObjectPtr<UBaseUIRoot> Root = GetRootLayout())
	{
		Root->PopLoadingOverlay();
	}
}

void UBaseMenuWidget::SwitchMenu(const EMainMenuType InType)
{
	if (const UMainUIRoot* Root = Cast<UMainUIRoot>(GetRootLayout()))
	{
		Root->PushMenu(InType); 
	}
}

TObjectPtr<UBaseUIRoot> UBaseMenuWidget::GetRootLayout() const
{
	const APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetLocalPlayer())
	{
		if (const ABaseHUD* MainHud = Cast<ABaseHUD>(PC->GetHUD()))
		{
			if (UBaseUIRoot* RootLayout = Cast<UBaseUIRoot>(MainHud->GetRootUI()))
			{
				return RootLayout;
			}
		}
	}
	return nullptr;
}
