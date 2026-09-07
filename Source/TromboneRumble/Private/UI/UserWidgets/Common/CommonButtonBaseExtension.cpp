// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonButtonBaseExtension.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "UI/Styles/CommonButtonStyleExtension.h"

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

void UCommonButtonBaseExtension::SetText(const FText& InText)
{
	if (Text_ActionName)
	{
		Text_ActionName->SetText(InText);
	}
}

void UCommonButtonBaseExtension::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	// The common single text is handled here, anything else by the blueprint
	const TSubclassOf<UCommonTextStyle> TextStyle = GetDesiredTextStyleClass();
	if (Text_ActionName)
	{
		Text_ActionName->SetStyle(TextStyle);
	}
	ApplyTextStyle(TextStyle);
}

void UCommonButtonBaseExtension::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (Image_Button)
	{
		Image_Button->SetBrush(ButtonBrush);
	}
}

void UCommonButtonBaseExtension::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshImageOpacity();
}

// Gamepad focus also lands here. CommonUI moves the hidden cursor onto the focused button and sends a real mouse move
void UCommonButtonBaseExtension::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	RefreshImageOpacity();
	PlayHoveredSound();
}

void UCommonButtonBaseExtension::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	
	RefreshImageOpacity();
}

void UCommonButtonBaseExtension::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	NativeOnCurrentTextStyleChanged();
	RefreshImageOpacity();
	PlayPressedSound();
}

void UCommonButtonBaseExtension::NativeOnReleased()
{
	Super::NativeOnReleased();
	
	NativeOnCurrentTextStyleChanged();
	RefreshImageOpacity();
}

void UCommonButtonBaseExtension::PlayHoveredSound() const
{
	if (const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO())
	{
		StyleExtension->PostHoveredSound(GetOwningPlayerPawn());
	}
}

void UCommonButtonBaseExtension::PlayPressedSound() const
{
	if (const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO())
	{
		StyleExtension->PostPressedSound(GetOwningPlayerPawn());
	}
}

void UCommonButtonBaseExtension::RefreshImageOpacity() const
{
	if (!Image_Button) return;
	
	const UCommonButtonStyleExtension* StyleExtension = GetStyleExtensionCDO();
	if (!StyleExtension) return;
	
	// Pressed wins over hovered. The cursor stays on the button while it is held
	float Opacity = StyleExtension->NormalImageOpacity;
	if (IsPressed())
	{
		Opacity = StyleExtension->PressedImageOpacity;
	}
	else if (IsHovered())
	{
		Opacity = StyleExtension->HoveredImageOpacity;
	}
	
	Image_Button->SetOpacity(Opacity);
}

const UCommonButtonStyleExtension* UCommonButtonBaseExtension::GetStyleExtensionCDO() const
{
	return Cast<UCommonButtonStyleExtension>(GetStyleCDO());
}
