#include "Subsystems/AppearanceSubsystem.h"
#include "EasySessionSubsystem.h"
#include "DeveloperSettings/TromboneConfig.h"

void UAppearanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	AvailableSkinColors = UTromboneConfig::Get()->CharacterSkinColors;
	UsedSkinColors.Empty();
	
	// The session subsystem initializes first, so its event exists to bind to.
	if (UEasySessionSubsystem* Sessions = Collection.InitializeDependency<UEasySessionSubsystem>())
	{
		Sessions->OnMatchmakingComplete.AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
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

void UAppearanceSubsystem::HandleMatchmakingComplete(EEasySessionResult Result, const FString& ErrorMessage)
{
	const UEasySessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UEasySessionSubsystem>();
	if (Result == EEasySessionResult::Success && Sessions && Sessions->IsHost())
	{
		ResetColors();
	}
}

void UAppearanceSubsystem::ResetColors()
{
	UsedSkinColors.Empty();
}