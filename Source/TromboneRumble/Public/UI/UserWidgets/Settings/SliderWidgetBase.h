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

	/** Recolors the progress bar for focus or hover. Focus counts on gamepad only, so a mouse click does not leave it lit. */
	void RefreshHighlight();

	/** Writes the fill color and the background tint of the progress bar. */
	void ApplyProgressBarColors(const FLinearColor& FillColor, const FLinearColor& BackgroundColor) const;

	/** Called with the new highlight state. Implement it for extra effects like an animation. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slider|Highlight")
	void ApplyHighlight(bool bInHighlighted);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	bool bHasValueText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	bool bHasTitleText = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Options")
	FText SliderTitle = FText::FromString(TEXT("Slider Title"));

	/** Progress bar fill color at rest. Applied on construct so this is the single source. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Highlight")
	FLinearColor NormalFillColor = FLinearColor::White;

	/** Progress bar fill color while focused or hovered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Highlight")
	FLinearColor HighlightedFillColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	/** Progress bar background tint at rest. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Highlight")
	FLinearColor NormalBackgroundColor = FLinearColor::White;

	/** Progress bar background tint while focused or hovered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slider|Highlight")
	FLinearColor HighlightedBackgroundColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	
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

	/** The inner slider is in the focus path. */
	bool bFocused = false;

	/** The pointer is over this widget. */
	bool bHovered = false;

	/** Last state handed to the progress bar, so the color is only set on change. */
	bool bHighlighted = false;

public:

	// ~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	// ~ End UUserWidget Interface
};