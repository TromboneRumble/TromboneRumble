#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"
#include "CommonTextBlock.h"
#include "UI/Styles/CommonButtonStyleExtension.h"

void UCommonButtonBaseWithText::SetText(const FText& InText) const
{
	if (CT_ButtonText)
	{
		CT_ButtonText->SetText(InText);
	}
}

void UCommonButtonBaseWithText::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	if (const UCommonButtonStyleExtension* ButtonStyleExtension = Cast<UCommonButtonStyleExtension>(GetStyleCDO()))
	{
		if (ButtonStyleExtension->NormalPressedTextStyle)
		{
			CT_ButtonText->SetStyle(ButtonStyleExtension->NormalPressedTextStyle);
			bIsNormalPressedTextStyleApplied = true;
		}
	}
}

void UCommonButtonBaseWithText::NativeOnReleased()
{
	Super::NativeOnReleased();
	
	if (bIsNormalPressedTextStyleApplied)
	{
		bIsNormalPressedTextStyleApplied = false;
		if (const TSubclassOf<UCommonTextStyle> CurrentTextStyle = GetCurrentTextStyleClass())
		{
			CT_ButtonText->SetStyle(CurrentTextStyle);
		}
	}
}