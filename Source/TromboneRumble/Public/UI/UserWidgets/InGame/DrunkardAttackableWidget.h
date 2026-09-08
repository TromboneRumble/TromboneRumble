// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UI/UserWidgets/Common/PopIndicatorWidget.h"
#include "DrunkardAttackableWidget.generated.h"

class UCommonActionWidget;
class UImage;
enum class ECommonInputType : uint8;

/** UDrunkardAttackableWidget
 *
 * Attackable mark on the drunkard. Shows the attack button glyph on gamepad and a plain icon on keyboard and mouse.
 */
UCLASS()
class TROMBONERUMBLE_API UDrunkardAttackableWidget : public UPopIndicatorWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** DT_Input row whose keys are drawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Input", meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle AttackActionRow;

private:

	/** Swaps between the icon and the glyph. Bound on every construct, the screen space component rebuilds this widget */
	void HandleInputMethodChanged(ECommonInputType NewInputType);

	/** Plain icon for keyboard and mouse. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	/** Glyph of the attack action. CommonUI redraws it and sets its own visibility when the input device changes. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> AttackActionGlyph;
};
