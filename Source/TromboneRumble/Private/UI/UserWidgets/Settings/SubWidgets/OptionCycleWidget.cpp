// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/SubWidgets/OptionCycleWidget.h"
#include "CommonRotator.h"
#include "CommonTextBlock.h"

void UOptionCycleWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (CT_OptionName) CT_OptionName->SetText(TextOptionName);
}

void UOptionCycleWidget::Init(const FText InName, const TArray<FText> InOptions, const int32 DefaultIndex)
{
	TextOptionName = InName;
	OptionsArray = InOptions;
	DefaultSelectedIndex = DefaultIndex;
	
	if (CT_OptionName) CT_OptionName->SetText(TextOptionName);
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(OptionsArray);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
}