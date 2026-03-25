#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PlayerNameplateWidget.generated.h"

class ADefaultPlayerState;

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

public:

	/** Initializes the player widget. */
	UFUNCTION(BlueprintCallable, Category = "Default")
	virtual void InitPlayerWidget(ADefaultPlayerState* InOwningPlayerState);

protected:

	/** Called when the owning player's name is changed. */
	UFUNCTION()
	virtual void OnPlayerNameChanged(const FString& PlayerName);

protected:

	/**
	 * Event when the player widget is initialized. Called after the native OnInitialized() event.
	 * At this point the owning player is set, and relevant events are bound.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Events", DisplayName = "On Player Widget Initialized")
	void K2_OnPlayerWidgetInitialized();

	/** Event when the owning player's name changes (including the first time it replicates). */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Events", DisplayName = "On Player Name Changed")
	void K2_OnPlayerNameChanged(const FString& PlayerName);
	
};
