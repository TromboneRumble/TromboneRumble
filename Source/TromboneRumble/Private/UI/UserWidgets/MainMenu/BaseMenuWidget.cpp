// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenu/BaseMenuWidget.h"
#include "EasySessionSubsystem.h"
#include "GameFramework/HUD.h"
#include "UI/HUD/MainHUD.h"
#include "UI/UserWidgets/MainMenu/MainUIRoot.h"
#include "UI/UserWidgets/MainMenu/NoticePopupWidget.h"

void UBaseMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Init();
	BindSubsystemCallbacks();
}

void UBaseMenuWidget::NativeDestruct()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	
	Super::NativeDestruct();
}

void UBaseMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

UWidget* UBaseMenuWidget::NativeGetDesiredFocusTarget() const
{
	return Super::NativeGetDesiredFocusTarget();
}

void UBaseMenuWidget::Init()
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
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

void UBaseMenuWidget::ChangeMenu(EMainMenuType InType) const
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
