#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PlayerNameplateWidget.generated.h"

class ADefaultPlayerState;
class UMatchPawnSpeakerWidget;

UCLASS()
class TROMBONERUMBLE_API UPlayerNameplateWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UPlayerNameplateWidget();
	
protected:

	/** Player state of the owning player. */
	UPROPERTY(Transient)
	ADefaultPlayerState* OwningPlayerState;

	/** Speaker indicator embedded in the nameplate. Optional so non-speaker nameplates still work. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UMatchPawnSpeakerWidget> SpeakerWidget;

public:

	/** Initializes the player widget. */
	UFUNCTION(BlueprintCallable, Category = "Nameplate")
	virtual void InitPlayerWidget(ADefaultPlayerState* InOwningPlayerState);

	/** @return Whether the owning player is the local player. */
	UFUNCTION(BlueprintPure, Category = "Nameplate")
	bool IsLocallyControlledPlayer() const;
	
	/** @return Whether the owning player is the host player. */
	UFUNCTION(BlueprintPure, Category = "Nameplate")
	bool IsHostPlayer() const;
	
protected:

	/** Called when the owning player's name is changed. */
	UFUNCTION()
	virtual void OnPlayerNameChanged(const FString& PlayerName);

protected:
	
	/** Event when the owning player's name changes (including the first time it replicates). */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Events", DisplayName = "On Player Name Changed")
	void K2_OnPlayerNameChanged(const FString& PlayerName);
	
};
