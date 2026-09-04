// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "Styling/SlateBrush.h"
#include "CommonRotatorWidgetBase.generated.h"

class UImage;

UCLASS()
class TROMBONERUMBLE_API UCommonRotatorWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	
	/** Locks the arrows so the selection cannot change. The label stays visible. */
	void SetInteractionEnabled(bool bInEnabled);
	
	/** Moves the rotator to the given index. */
	void SetSelectedIndex(int32 NewIndex);
	
	/** Replaces the labels. */
	void SetOptions(const TArray<FText>& InOptions);
	
	/** Replaces the labels and their backgrounds. Arrays pair up by index. */
	void SetOptions(const TArray<FText>& InOptions, const TArray<FSlateBrush>& InBackgrounds);
	
	/** @return Index of the label shown now, or -1 without a rotator. */
	int32 GetCurrentIndex() const { return CR_Rotator ? CR_Rotator->GetSelectedIndex() : -1; }
	
	/** @return All labels, in display order. */
	const TArray<FText>& GetOptions() const { return TextOptions; }
	
public:
	
	/** Delegate for when the rotator is rotated with a direction. Provides the new index and the direction of rotation. */
	FOnRotatedWithDirection& OnRotatedWithDirection() const { return CR_Rotator->OnRotatedWithDirection; }

	/** @return The inner widget that should receive gamepad/keyboard focus. The rotator handles nav left/right natively while focused. */
	UWidget* GetFocusWidget() const { return CR_Rotator; }
	
protected:
	
	/** Labels shown on the rotator, in order. */
	UPROPERTY(EditAnywhere, Category = "Options")
	TArray<FText> TextOptions;
	
	/** Which label is shown first. */
	UPROPERTY(EditAnywhere, Category = "Options")
	int32 DefaultSelectedIndex = 0;
	
	/** Paired with TextOptions by index. A missing or empty entry means no background for that label. */
	UPROPERTY(EditAnywhere, Category = "Options")
	TArray<FSlateBrush> BackgroundBrushes;

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonRotator> CR_Rotator;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Prev;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Next;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Background;
	
private:
	
	/** Hooks the prev/next buttons up to the rotator. */
	void BindButtonEvents();
	
	/** Pushes the current labels into the rotator and resets the selection. */
	void RefreshRotator();
	
	/** Shows the background for the current label, or hides it when the label has none. */
	void ApplyBackground();
	
	/** Called when the rotator turns, so the background follows. */
	UFUNCTION()
	void HandleRotatedForBackground(int32 NewIndex, ERotatorDirection Direction);
	
public:
	
	//~ Begin UUserWidget Interface
	virtual bool Initialize() override;
	//~ End UUserWidget Interface
	
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
	
protected:
	
	//~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	//~ End UUserWidget Interface

};
