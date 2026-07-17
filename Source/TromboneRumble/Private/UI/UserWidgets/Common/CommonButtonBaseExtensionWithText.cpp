// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"
#include "AkGameplayStatics.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CommonTextBlock.h"
#include "UI/Styles/CommonButtonStyleExtension.h"

void UCommonButtonBaseExtensionWithText::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	// Mouse clicks also move focus; only mirror hover feedback for gamepad-driven focus to avoid double audio
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		PlayHoverFeedback();
	}
}

void UCommonButtonBaseExtensionWithText::SetText(const FText& InText) const
{
	if (Text_ActionName)
	{
		Text_ActionName->SetText(InText);
	}
}

void UCommonButtonBaseExtensionWithText::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedButtonStyleExtension = Cast<UCommonButtonStyleExtension>(GetStyleCDO());
}

void UCommonButtonBaseExtensionWithText::NativeOnHovered()
{
	Super::NativeOnHovered();

	PlayHoverFeedback();
}

void UCommonButtonBaseExtensionWithText::PlayHoverFeedback() const
{
	if (CachedButtonStyleExtension && CachedButtonStyleExtension->NormalHoveredAudioEvent)
	{
		UAkGameplayStatics::PostEvent(CachedButtonStyleExtension->NormalHoveredAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
	}
}

void UCommonButtonBaseExtensionWithText::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	if (CachedButtonStyleExtension && CachedButtonStyleExtension->NormalPressedTextStyle && bUseText)
	{
		Text_ActionName->SetStyle(CachedButtonStyleExtension->NormalPressedTextStyle);
		bIsNormalPressedTextStyleApplied = true;
	}
	
	if (CachedButtonStyleExtension && CachedButtonStyleExtension->NormalPressedAudioEvent)
	{
		UAkGameplayStatics::PostEvent(CachedButtonStyleExtension->NormalPressedAudioEvent, GetOwningPlayerPawn(), 0, FOnAkPostEventCallback());
	}
}

void UCommonButtonBaseExtensionWithText::NativeOnReleased()
{
	Super::NativeOnReleased();
	
	if (bIsNormalPressedTextStyleApplied && bUseText)
	{
		bIsNormalPressedTextStyleApplied = false;
		if (const TSubclassOf<UCommonTextStyle> CurrentTextStyle = GetCurrentTextStyleClass())
		{
			Text_ActionName->SetStyle(CurrentTextStyle);
		}
	}
}