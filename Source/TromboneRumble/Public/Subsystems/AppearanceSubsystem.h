#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AppearanceSubsystem.generated.h"

/* 
 * Managing player’s unique skin color
 */
UCLASS()
class TROMBONERUMBLE_API UAppearanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Returns a unique color for the player. If all colors are used, returns a default color (e.g., gray). */
	FLinearColor AssignUniqueColor();

	/** Releases a color back to the pool of available colors. Should be called when a player leaves the game. */
	void ReleaseColor(const FLinearColor& Color);

private:
	
	/** List of available skin colors */
	UPROPERTY()
	TArray<FLinearColor> AvailableSkinColors;

	/** List of currently used skin colors */
	UPROPERTY()
	TArray<FLinearColor> UsedSkinColors;
	
};
