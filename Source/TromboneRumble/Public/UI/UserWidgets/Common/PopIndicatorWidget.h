// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PopIndicatorWidget.generated.h"

/** UPopIndicatorWidget
 *
 * Small indicator that pops in and shrinks out with UTweenSubsystem.
 */
UCLASS()
class TROMBONERUMBLE_API UPopIndicatorWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/** Pops in when true, shrinks out when false. True while already shown pops again. */
	void SetShown(bool bInShown);
	bool IsShown() const { return bShown; }

protected:

	virtual void NativeConstruct() override;

	/** Applies the current shown state without playing anything. Runs on every construct. Derived classes add their look here. */
	virtual void ApplyShownState();

	/** Seconds for the pop from scale 0 to 1 with overshoot. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Tween", meta = (ClampMin = "0.01"))
	float AppearDuration = 0.25f;

	/** Seconds for the shrink to scale 0. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Tween", meta = (ClampMin = "0.01"))
	float HideDuration = 0.15f;

private:

	/** Hide tween finished. Drops the opacity so the last frame does not linger. */
	void HandleHideFinished();

	bool bShown = false;
};
