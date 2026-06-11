// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "GameFramework/HUD.h"
#include "UI/HUD/MainHUD.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"

void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Init();
}

void UBaseMenuWidget::Init()
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
