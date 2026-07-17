// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SliderWidgetBase.generated.h"

class UProgressBar;
class UCommonTextBlock;
class UAnalogSlider;

UCLASS()
class TROMBONERUMBLE_API USliderWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	
	/** Initializes the slider widget with slider value change callback. */
	virtual void Init(TFunction<void(float)> OnValueChangedCallback = nullptr);
	
	/** Sets the slider value. Value should be between 0.0 and 1.0. */
	void SetValue(float InValue) const;
	
	/** @return Current slider value. */
	float GetValue() const;

	/** @return The inner widget that should receive gamepad/keyboard focus. The analog slider adjusts by StepSize on nav left/right while focused. */
	UWidget* GetFocusWidget() const;

protected:
	
	UFUNCTION()
	virtual void OnSliderValueChanged(float Value);
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	bool bHasValueText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	bool bHasTitleText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	FText SliderTitle = FText::FromString(TEXT("Slider Title"));
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UAnalogSlider> Slider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Text_SliderValue;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Text_SliderTitle;
	
private:
	
	TFunction<void(float)> OnValueChanged;
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UUserWidget Interface
};