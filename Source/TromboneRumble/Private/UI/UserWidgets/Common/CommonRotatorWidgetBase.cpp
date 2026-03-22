// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"

bool UCommonRotatorWidgetBase::Initialize()
{
	if (Super::Initialize())
	{
		InitButtons();
		
		return true;
	}
	
	return false;
}

void UCommonRotatorWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(OptionsArray);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
}

void UCommonRotatorWidgetBase::Init(const TArray<FText> InOptions, const int32 InDefaultIndex)
{
	OptionsArray = InOptions;
	DefaultSelectedIndex = InDefaultIndex;
	
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(OptionsArray);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
}

void UCommonRotatorWidgetBase::SetIsEnabled(const bool bInIsEnabled)
{
	if (CB_Prev)
	{
		CB_Prev->SetIsEnabled(bInIsEnabled);
		CB_Prev->SetVisibility(bInIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);		
	}
	if (CB_Next)
	{
		CB_Next->SetIsEnabled(bInIsEnabled);
		CB_Next->SetVisibility(bInIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UCommonRotatorWidgetBase::SetSelectedIndex(int32 NewIndex)
{
	if (!OptionsArray.IsValidIndex(NewIndex)) 
	{
		NewIndex = (DefaultSelectedIndex != -1) ? DefaultSelectedIndex : 0;
	}

	if (CR_Rotator)
	{
		CR_Rotator->SetSelectedItem(NewIndex);
	}
}

void UCommonRotatorWidgetBase::InitButtons()
{
	if (CR_Rotator)
	{
		if (CB_Prev)
		{
			CB_Prev->OnClicked().AddLambda([this]
			{
				CR_Rotator->ShiftTextLeft();
				OnOptionChanged.Broadcast(GetCurrentIndex());
			});
		}
		if (CB_Next)
		{
			CB_Next->OnClicked().AddLambda([this]
			{
				CR_Rotator->ShiftTextRight();
				OnOptionChanged.Broadcast(GetCurrentIndex());
			});
		}
	}
}