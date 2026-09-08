// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/PopIndicatorWidget.h"
#include "DrunkardTargetWidget.generated.h"

class UImage;

/** UDrunkardTargetWidget
 *
 * Portrait above the drunkard's head, tinted with the target's skin color.
 */
UCLASS()
class TROMBONERUMBLE_API UDrunkardTargetWidget : public UPopIndicatorWidget
{
	GENERATED_BODY()

public:

	/** Alpha 0 hides the portrait. Any other color tints it and pops it in, again when the color changes. */
	void SetTargetColor(const FLinearColor& InColor);

protected:

	//~ Begin UPopIndicatorWidget Interface
	virtual void ApplyShownState() override;
	//~ End UPopIndicatorWidget Interface

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PortraitBackground;

	FLinearColor TargetColor = FLinearColor::Transparent;
};
