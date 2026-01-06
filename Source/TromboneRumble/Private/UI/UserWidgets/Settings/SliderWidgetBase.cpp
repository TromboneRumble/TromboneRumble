// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Settings/SliderWidgetBase.h"
#include "AnalogSlider.h"
#include "CommonTextBlock.h"

void USliderWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (bHasValueText && Text_SliderValue)
	{
		Text_SliderValue->SetVisibility(ESlateVisibility::Visible);
		Text_SliderValue->SetText(FText::AsNumber(FMath::RoundToInt(GetValue() * 100.0f)));
	}
	else if (Text_SliderValue)
	{
		Text_SliderValue->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (bHasTitleText && Text_SliderTitle)
	{
		Text_SliderTitle->SetVisibility(ESlateVisibility::Visible);
		Text_SliderTitle->SetText(SliderTitle);
	}
	else if (Text_SliderTitle)
	{
		Text_SliderTitle->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USliderWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	Init();
}

void USliderWidgetBase::Init(TFunction<void(float)> OnValueChangedCallback)
{
	OnValueChanged = OnValueChangedCallback;
	
	if (Slider)
	{
		Slider->OnValueChanged.AddDynamic(this, &ThisClass::OnSliderValueChanged);
	}
}

void USliderWidgetBase::SetValue(const float InValue) const
{
	if (Slider)
	{
		Slider->SetValue(InValue);
	}
	Text_SliderValue->SetText(FText::AsNumber(FMath::RoundToInt(InValue * 100.0f)));
}

float USliderWidgetBase::GetValue() const
{
	return Slider ? Slider->GetValue() : 0.0f;
}

void USliderWidgetBase::OnSliderValueChanged(const float Value)
{
	if (bHasValueText && Text_SliderValue)
	{
		Text_SliderValue->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100.0f)));
	}
	
	if (OnValueChanged)
	{
		OnValueChanged(Value);
	}
}