// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/InGame/DrunkardTargetWidget.h"
#include "Components/Image.h"

void UDrunkardTargetWidget::SetTargetColor(const FLinearColor& InColor)
{
	TargetColor = InColor;
	SetShown(InColor.A > 0.f);
}

void UDrunkardTargetWidget::ApplyShownState()
{
	Super::ApplyShownState();

	if (PortraitBackground && IsShown())
	{
		PortraitBackground->SetColorAndOpacity(TargetColor);
	}
}
