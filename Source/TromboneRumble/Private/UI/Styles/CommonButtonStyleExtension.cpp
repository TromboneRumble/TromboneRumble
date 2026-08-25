// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/Styles/CommonButtonStyleExtension.h"
#include "AkGameplayStatics.h"

void UCommonButtonStyleExtension::PostHoveredSound(AActor* SoundOwner) const
{
	if (NormalHoveredAudioEvent)
	{
		UAkGameplayStatics::PostEvent(NormalHoveredAudioEvent, SoundOwner, 0, FOnAkPostEventCallback());
	}
}

void UCommonButtonStyleExtension::PostPressedSound(AActor* SoundOwner) const
{
	if (NormalPressedAudioEvent)
	{
		UAkGameplayStatics::PostEvent(NormalPressedAudioEvent, SoundOwner, 0, FOnAkPostEventCallback());
	}
}
