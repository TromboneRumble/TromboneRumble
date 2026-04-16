#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/UserWidgets/Popup/EscapePopup.h"
#include "UI/UserWidgets/Popup/NoticePopupWidget.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "UI/UserWidgets/Settings/SettingPopup.h"
#include "TromboneConfig.generated.h"

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
	
	
	/** @return Popup widget class of type T */
	template<typename T>
	TSubclassOf<T> GetPopupClass() const
	{
		if (T::StaticClass()->IsChildOf(UNoticePopupWidget::StaticClass()))
		{
			return Cast<UClass>(NoticePopupWidgetClass);
		}
    
		if (T::StaticClass()->IsChildOf(UTwoButtonWithoutClosePopup::StaticClass()))
		{
			return Cast<UClass>(TwoButtonWithoutClosePopupWidgetClass);
		}
		
		if (T::StaticClass()->IsChildOf(UEscapePopup::StaticClass()))
		{
			return Cast<UClass>(EscapePopupWidgetClass);
		}
		
		if (T::StaticClass()->IsChildOf(USettingPopup::StaticClass()))
		{
			return Cast<UClass>(SettingPopupWidgetClass);
		}

		UE_LOG(LogTemp, Error, TEXT("No matching popup class found for type %s. Please check if it's added in UTromboneConfig."), *T::StaticClass()->GetName());
		return nullptr;
	}
	
public:
	
	/** Notice popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UNoticePopupWidget> NoticePopupWidgetClass;
	
	/** Two-button without close button popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UTwoButtonWithoutClosePopup> TwoButtonWithoutClosePopupWidgetClass;
	
	/** Escape popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEscapePopup> EscapePopupWidgetClass;
	
	/** Setting popup widget class. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USettingPopup> SettingPopupWidgetClass;
	
public:
	
	/** Skin color randomly assigned to a character in-game */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TArray<FLinearColor> CharacterSkinColors;
	
public:
	
	/** Time in seconds for the lobby countdown before server travel. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Lobby")
	int32 LobbyCountdownTimeSeconds;
	
public:
	
	/** Tutorial data table for the tutorial sequence. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Tutorial")
	TSoftObjectPtr<UDataTable> TutorialDataTable;

	/** Quest data table for the tutorial. */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Tutorial")
	TSoftObjectPtr<UDataTable> QuestDataTable;
	
	/** Interval after quest completion before processing the next tutorial sequence (seconds) */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Tutorial", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float IntervalAfterQuestCompletion;
	
	/** Initial delay before starting the tutorial (seconds) */
	UPROPERTY(Config, NoClear, EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Tutorial", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float TutorialStartDelay;
	
};