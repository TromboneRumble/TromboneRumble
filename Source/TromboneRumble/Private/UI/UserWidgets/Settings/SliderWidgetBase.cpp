#include "UI/UserWidgets/Settings/SliderWidgetBase.h"
#include "AnalogSlider.h"
#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"

UWidget* USliderWidgetBase::GetFocusWidget() const
{
	return Slider;
}

void USliderWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (Text_SliderValue)
	{
		Text_SliderValue->SetVisibility(bHasValueText ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Text_SliderValue->SetText(FText::AsNumber(FMath::RoundToInt(GetValue() * 100.0f)));
	}
	
	if (Text_SliderTitle)
	{
		Text_SliderTitle->SetVisibility(bHasTitleText ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Text_SliderTitle->SetText(SliderTitle);
	}
}

void USliderWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Slider)
	{
		Slider->OnValueChanged.RemoveAll(this);
		Slider->OnValueChanged.AddDynamic(this, &ThisClass::OnSliderValueChanged);
	}
}

void USliderWidgetBase::NativeDestruct()
{
	if (Slider)
	{
		Slider->OnValueChanged.RemoveAll(this);
	}
	OnValueChanged = nullptr;
	
	Super::NativeDestruct();
}

void USliderWidgetBase::Init(TFunction<void(float)> OnValueChangedCallback)
{
	OnValueChanged = OnValueChangedCallback;
}

void USliderWidgetBase::SetValue(const float InValue) const
{
	if (Slider)
	{
		Slider->SetValue(FMath::Clamp(InValue, 0.0f, 1.0f));
	}
	
	if (bHasValueText && Text_SliderValue)
	{
		Text_SliderValue->SetText(FText::AsNumber(FMath::RoundToInt(InValue * 100.0f)));
	}
	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(InValue, 0.0f, 1.0f));
	}
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
	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(Value, 0.0f, 1.0f));
	}
	
	if (OnValueChanged)
	{
		OnValueChanged(Value);
	}
}