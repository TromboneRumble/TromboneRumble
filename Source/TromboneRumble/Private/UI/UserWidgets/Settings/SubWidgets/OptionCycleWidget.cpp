// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleWidget.h"
#include "CommonButtonBase.h"
#include "CommonRotator.h"
#include "CommonTextBlock.h"

void UOptionCycleWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (CT_OptionName) CT_OptionName->SetText(TextOptionName);
	if (WBP_OptionRotator)
	{
		WBP_OptionRotator->PopulateTextLabels(OptionsArray);
		WBP_OptionRotator->SetSelectedItem(DefaultSelectedIndex);
	}
}

void UOptionCycleWidget::Init(const FText InName, const TArray<FText> InOptions, const int32 DefaultIndex)
{
	TextOptionName = InName;
	OptionsArray = InOptions;
	DefaultSelectedIndex = DefaultIndex;
	
	if (CT_OptionName) CT_OptionName->SetText(TextOptionName);
	if (WBP_OptionRotator)
	{
		WBP_OptionRotator->PopulateTextLabels(OptionsArray);
		WBP_OptionRotator->SetSelectedItem(DefaultSelectedIndex);
	}
	
	InitButtons();
}

void UOptionCycleWidget::InitButtons()
{
	if (CB_Prev && WBP_OptionRotator)
	{
		CB_Prev->OnClicked().AddLambda([this]
		{
			WBP_OptionRotator->ShiftTextLeft();
		});
	}
	if (CB_Next && WBP_OptionRotator)
	{
		CB_Next->OnClicked().AddLambda([this]
		{
			WBP_OptionRotator->ShiftTextRight();
		});
	}
}