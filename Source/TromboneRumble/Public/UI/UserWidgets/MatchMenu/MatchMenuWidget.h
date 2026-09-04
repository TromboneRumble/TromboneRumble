// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Utilities/Defines.h"
#include "MatchMenuWidget.generated.h"

class AMatchMenuGameState;
class UDataTable;
class UCommonButtonBaseExtensionWithText;
class UCommonRotatorWidgetBase;
enum class ERotatorDirection : uint8;
enum class EMatchType : uint8;

UCLASS()
class TROMBONERUMBLE_API UMatchMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

protected:
	
	//~ Begin UIs
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBaseExtensionWithText> CB_Start;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonButtonBaseExtensionWithText> CB_Back;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonRotatorWidgetBase> CR_MatchType;
	
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "UI")
	TObjectPtr<UCommonRotatorWidgetBase> CR_Map;
	//~ End UIs
	
	/** Fills the match type rotator (FMatchTypeRow). Required - an empty table leaves the rotator empty. */
	UPROPERTY(EditDefaultsOnly, Category = "Config", meta = (RequiredAssetDataTags = "RowStructure=/Script/TromboneRumble.MatchTypeRow"))
	TObjectPtr<UDataTable> MatchTypeTable;
	
	/** Fills the map rotator (FSelectableMapRow). Which maps exist comes from GameMapDeveloperSettings. */
	UPROPERTY(EditDefaultsOnly, Category = "Config", meta = (RequiredAssetDataTags = "RowStructure=/Script/TromboneRumble.SelectableMapRow"))
	TObjectPtr<UDataTable> SelectableMapTable;
	
private:
	
	//~ Begin GameState Events
	/** Moves the match type rotator to what the game state says. */
	UFUNCTION()
	void OnMatchTypeChanged(EMatchType NewType);
	
	/** Moves the map rotator to what the game state says. */
	UFUNCTION()
	void OnSelectedMapChanged(FGameplayTag NewMapTag);
	//~ End GameState Events
	
	//~ Begin UI Events
	/** Starts the match and travels everyone to the picked lobby. */
	UFUNCTION()
	void HandleStartButtonClicked();
	
	/** Leaves the session and returns to the main menu. */
	UFUNCTION()
	void HandleBackButtonClicked();
	
	/** Host turned the match type rotator. Writes the choice to the game state and the session. */
	UFUNCTION()
	void HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir);
	
	/** Host turned the map rotator. Writes the choice to the game state and the session. */
	UFUNCTION()
	void HandleOnRotatedMap(int32 Value, ERotatorDirection RotatorDir);
	
	/** Clears the loading overlay once the session update ends. */
	UFUNCTION()
	void HandleOnUpdateMatchComplete(bool bWasSuccessful);
	//~ End UI Events
	
	/** Fills the map rotator from the table. */
	void InitSelectableMaps();
	
	/** Fills the match type rotator from the table. */
	void InitMatchTypes();
	
private:
	
	/** Blocks every input once the match is starting. */
	bool bIsStarted = false;
	
	/** Map tags in rotator order. Index conversion must go through this array. */
	UPROPERTY(Transient)
	TArray<FGameplayTag> CachedSelectableMaps;
	
	/** Match types in rotator order. Index conversion must go through this array. */
	UPROPERTY(Transient)
	TArray<EMatchType> CachedMatchTypes;
	
	/** The game state this menu listens to. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AMatchMenuGameState> CachedMatchMenuGS;
	
public:
	
	//~ Begin UCommonActivatableWidget Interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture, EMouseLockMode::LockOnCapture, false);
	}
	//~ End UCommonActivatableWidget Interface
	
	//~ Begin UBaseMenuWidget Interface
	virtual void Init() override;
	virtual void SetUIEnabled(const bool bEnabled) override;
	//~ End UBaseMenuWidget Interface
	
protected:
	
	//~ Begin UCommonActivatableWidget Interface
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~ End UCommonActivatableWidget Interface
	
	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface

};