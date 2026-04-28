#include "UI/UserWidgets/Common/CommonButtonBaseWithText.h"

#include "AkGameplayStatics.h"
#include "CommonTextBlock.h"
#include "UI/Styles/CommonButtonStyleExtension.h"

void UCommonButtonBaseWithText::SetText(const FText& InText) const
{
	if (CT_ButtonText)
	{
		CT_ButtonText->SetText(InText);
	}
}

void UCommonButtonBaseWithText::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	if (const UCommonButtonStyleExtension* ButtonStyleExtension = Cast<UCommonButtonStyleExtension>(GetStyleCDO()))
	{
		if (ButtonStyleExtension->HoveredAudioEvent)
		{
			UAkGameplayStatics::PostEvent(ButtonStyleExtension->HoveredAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
		}
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
		
		if (ButtonStyleExtension->NormalPressedAudioEvent)
		{
			UAkGameplayStatics::PostEvent(ButtonStyleExtension->NormalPressedAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
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