#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TromboneConfig.generated.h"

enum class EWeaponType : uint8;
/**
 * Config for Trombone Rumble Project.
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Trombone Config"))
class TROMBONERUMBLE_API UTromboneConfig : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UTromboneConfig();

	/**
	 * Get the global Trombone UI settings object.
	 */
	static const UTromboneConfig* Get();
	
public:
	
	/** Notice popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UNoticePopupWidget> NoticePopupWidgetClass;
	
	/** Two-button without close button popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UTwoButtonWithoutClosePopup> TwoButtonWithoutClosePopupWidgetClass;
	
public:
	/** Skin color randomly assigned to a character in-game */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TArray<FLinearColor> CharacterSkinColors;
	
};