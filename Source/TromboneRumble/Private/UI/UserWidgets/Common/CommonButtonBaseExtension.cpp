// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonButtonBaseExtension.h"
#include "CommonTextBlock.h"
#include "UI/Styles/CommonButtonStyleExtension.h"

void UCommonButtonBaseExtension::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	if (const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO())
	{
		StyleExtension->PostHoveredSound(GetOwningPlayerPawn());
	}
}

void UCommonButtonBaseExtension::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	NativeOnCurrentTextStyleChanged();
	
	if (const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO())
	{
		StyleExtension->PostPressedSound(GetOwningPlayerPawn());
	}
}

void UCommonButtonBaseExtension::NativeOnReleased()
{
	Super::NativeOnReleased();
	
	NativeOnCurrentTextStyleChanged();
}

const UCommonButtonStyleExtension* UCommonButtonBaseExtension::GetStyleExtensionCDO() const
{
	return Cast<UCommonButtonStyleExtension>(GetStyleCDO());
}

TSubclassOf<UCommonTextStyle> UCommonButtonBaseExtension::GetDesiredTextStyleClass() const
{
	if (IsPressed())
	{
		if (const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO())
		{
			if (StyleExtension->NormalPressedTextStyle)
			{
				return StyleExtension->NormalPressedTextStyle;
			}
		}
	}
	
	return GetCurrentTextStyleClass();
}
