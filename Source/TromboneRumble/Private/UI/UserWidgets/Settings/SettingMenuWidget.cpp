// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/SettingMenuWidget.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"

UWidget* USettingMenuWidget::NativeGetDesiredFocusTarget() const
{
	if (CB_Audio)
	{
		return CB_Audio;
	}
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
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddUObject(this, &USettingMenuWidget::DeactivateWidget);
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