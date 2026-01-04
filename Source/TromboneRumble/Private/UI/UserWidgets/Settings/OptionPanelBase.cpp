// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/OptionPanelBase.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "CommonButtonBase.h"

void UOptionPanelBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitButtons();
}

void UOptionPanelBase::Init(const TFunction<void()> BackAction)
{
	OnBackAction = BackAction;
	SaveManagerSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
}

void UOptionPanelBase::InitButtons()
{
	if (Button_Back)
	{
		Button_Back->OnClicked().AddUObject(this, &ThisClass::HandleBackButtonClicked);
	}
}

void UOptionPanelBase::HandleBackButtonClicked()
{
	if (OnBackAction)
	{
		OnBackAction();
	}
}

void UOptionPanelBase::HandleApplyButtonClicked()
{
}

void UOptionPanelBase::HandleResetButtonClicked()
{
}
