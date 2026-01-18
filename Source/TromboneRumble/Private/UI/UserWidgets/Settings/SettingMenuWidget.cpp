// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/SettingMenuWidget.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"

void USettingMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void USettingMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USettingMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void USettingMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

UWidget* USettingMenuWidget::NativeGetDesiredFocusTarget() const
{
	return Super::NativeGetDesiredFocusTarget();
}

void USettingMenuWidget::Init()
{
	Super::Init();
	
	if (CB_Audio)
	{
		CB_Audio->OnClicked().RemoveAll(this);
		CB_Audio->OnClicked().AddLambda([this] { ChangePanel(Widget_AudioOptions); });
	}
	if (CB_Video)
	{
		CB_Video->OnClicked().RemoveAll(this);
		CB_Video->OnClicked().AddLambda([this] { ChangePanel(Widget_VideoOptions); });
	}
	if (Widget_AudioOptions)
	{
		Widget_AudioOptions->Init([this] { ChangePanel(VB_Settings); });
	}
	if (Widget_VideoOptions)
	{
		Widget_VideoOptions->Init([this] { ChangePanel(VB_Settings); });
	}
}

void USettingMenuWidget::ChangePanel(UWidget* TargetWidget)
{
	if (CAS_Settings)
	{
		CAS_Settings->SetActiveWidget(TargetWidget);
	}	
}
