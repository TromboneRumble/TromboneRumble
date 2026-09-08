// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DrunkardTargetWidget.generated.h"

class UImage;

/** UDrunkardTargetWidget
 *
 * Portrait above the drunkard's head, tinted with the target's skin color.
 */
UCLASS()
class TROMBONERUMBLE_API UDrunkardTargetWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	
	/** Alpha 0 hides the portrait. Any other color tints it and pops it in, again when the color changes. */
	void SetTargetColor(const FLinearColor& InColor);

protected:
	
	virtual void NativeConstruct() override;

	/** Seconds for the pop from scale 0 to 1 with overshoot. */
	UPROPERTY(EditDefaultsOnly, Category = "Target|Tween", meta = (ClampMin = "0.01"))
	float AppearDuration = 0.25f;

	/** Seconds for the shrink to scale 0. */
	UPROPERTY(EditDefaultsOnly, Category = "Target|Tween", meta = (ClampMin = "0.01"))
	float HideDuration = 0.15f;

private:
	
	/** Puts the widget in its current shown state without playing anything. Runs on every construct. */
	void ApplyShownState();

	/** Hide tween finished. Drops the opacity so the last frame does not linger. */
	void HandleHideFinished();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PortraitBackground;

	FLinearColor TargetColor = FLinearColor::Transparent;
	bool bShown = false;
};
