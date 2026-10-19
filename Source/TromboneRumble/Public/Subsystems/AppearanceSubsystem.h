#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AppearanceSubsystem.generated.h"

enum class EEasySessionResult : uint8;

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
	
	/** Resets the color management system, clearing all used colors. Useful for starting a new game session. */
	void ResetColors();
	
protected:
	
	/** Called when matchmaking is complete. A run that ended hosting a new room starts with every color free. */
	UFUNCTION()
	void HandleMatchmakingComplete(EEasySessionResult Result, const FString& ErrorMessage);
	
private:
	
	/** List of available skin colors */
	UPROPERTY()
	TArray<FLinearColor> AvailableSkinColors;

	/** List of currently used skin colors */
	UPROPERTY()
	TArray<FLinearColor> UsedSkinColors;
	
};
