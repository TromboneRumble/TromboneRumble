// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/BaseMenuWidget.h"
#include "EasySessionSubsystem.h"
#include "GameFramework/HUD.h"
#include "UI/HUD/MainHUD.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "UI/UserWidgets/MainMenu/NoticePopupWidget.h"

void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Init();
	BindSubsystemCallbacks();
}

void UBaseMenuWidget::Init()
{
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
}

void UBaseMenuWidget::ShowNoticePopup(const FString& Content)
{
	if (NoticePopupWidgetClass)
	{
		UNoticePopupWidget* NoticePopup = CreateWidget<UNoticePopupWidget>(GetOwningPlayer(), NoticePopupWidgetClass);
		NoticePopup->OnInit(Content);
	}
}

void UBaseMenuWidget::BindSubsystemCallbacks()
{
	if (!SessionsSubsystem)
	{
		const UGameInstance* GameInstance = GetGameInstance();
		SessionsSubsystem = GameInstance->GetSubsystem<UEasySessionSubsystem>();
	}
	
	RemoveSubsystemCallbacks();
}

void UBaseMenuWidget::RemoveSubsystemCallbacks()
{
}

void UBaseMenuWidget::SetUIEnabled(const bool bEnabled)
{
}

void UBaseMenuWidget::SwitchMenu(EMainMenuType InType)
{
	if (const UMainUIRoot* Root = GetRootLayout())
	{
		Root->PushMenu(InType); 
	}
}

TObjectPtr<UMainUIRoot> UBaseMenuWidget::GetRootLayout() const
{
	const APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetLocalPlayer())
	{
		if (const AMainHUD* MainHud = Cast<AMainHUD>(PC->GetHUD()))
		{
			if (UMainUIRoot* RootLayout = Cast<UMainUIRoot>(MainHud->GetRootLayout()))
			{
				return RootLayout;
			}
		}
	}
	return nullptr;
}
