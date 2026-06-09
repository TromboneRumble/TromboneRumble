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

void UCommonButtonBaseWithText::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedButtonStyleExtension = Cast<UCommonButtonStyleExtension>(GetStyleCDO());
}

void UCommonButtonBaseWithText::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	if (CachedButtonStyleExtension && CachedButtonStyleExtension->HoveredAudioEvent)
	{
		UAkGameplayStatics::PostEvent(CachedButtonStyleExtension->HoveredAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
	}
}

void UCommonButtonBaseWithText::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	if (CachedButtonStyleExtension && CachedButtonStyleExtension->NormalPressedTextStyle)
	{
		CT_ButtonText->SetStyle(CachedButtonStyleExtension->NormalPressedTextStyle);
		bIsNormalPressedTextStyleApplied = true;
	}

	if (CachedButtonStyleExtension && CachedButtonStyleExtension->NormalPressedAudioEvent)
	{
		UAkGameplayStatics::PostEvent(CachedButtonStyleExtension->NormalPressedAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
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