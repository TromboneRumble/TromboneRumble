// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SliderWidgetBase.generated.h"

class UCommonTextBlock;
class UAnalogSlider;

UCLASS()
class TROMBONERUMBLE_API USliderWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	virtual void Init(TFunction<void(float)> OnValueChangedCallback = nullptr);
	
	void SetValue(float InValue) const;
	float GetValue() const;
	
protected:
	UFUNCTION()
	virtual void OnSliderValueChanged(float Value);
	
	TFunction<void(float)> OnValueChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasValueText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasTitleText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SliderTitle = FText::FromString(TEXT("Slider Title"));
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAnalogSlider> Slider;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Text_SliderValue;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Text_SliderTitle;
};