#include "Subsystems/AppearanceSubsystem.h"
#include "DeveloperSettings/TromboneConfig.h"

void UAppearanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	AvailableSkinColors = UTromboneConfig::Get()->CharacterSkinColors;
}

FLinearColor UAppearanceSubsystem::AssignUniqueColor()
{
	for (const FLinearColor& Color : AvailableSkinColors)
	{
		if (!UsedSkinColors.Contains(Color))
		{
			UsedSkinColors.Add(Color);
			return Color;
		}
	}

	return FLinearColor::Gray;
}

void UAppearanceSubsystem::ReleaseColor(const FLinearColor& Color)
{
	UsedSkinColors.Remove(Color);
}