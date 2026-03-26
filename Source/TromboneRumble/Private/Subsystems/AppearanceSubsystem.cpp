#include "Subsystems/AppearanceSubsystem.h"
#include "EasyMatchmakingManager.h"
#include "EasySessionTypes.h"
#include "DeveloperSettings/TromboneConfig.h"

void UAppearanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	AvailableSkinColors = UTromboneConfig::Get()->CharacterSkinColors;
	UsedSkinColors.Empty();
	
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
	}
}

FLinearColor UAppearanceSubsystem::AssignUniqueColor()
{
	TArray<FLinearColor> RemainingColors;
	for (const FLinearColor& Color : AvailableSkinColors)
	{
		if (!UsedSkinColors.Contains(Color))
		{
			RemainingColors.Add(Color);
		}
	}

	if (RemainingColors.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, RemainingColors.Num() - 1);
		const FLinearColor ChosenColor = RemainingColors[RandomIndex];

		UsedSkinColors.Add(ChosenColor);
		return ChosenColor;
	}

	return FLinearColor::Gray;
}

void UAppearanceSubsystem::ReleaseColor(const FLinearColor& Color)
{
	UsedSkinColors.Remove(Color);
}

void UAppearanceSubsystem::HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result)
{
	if (Result == EEasyMatchmakingCompleteResult::SessionCreated)
	{
		ResetColors();
	}
}

void UAppearanceSubsystem::ResetColors()
{
	UsedSkinColors.Empty();
}