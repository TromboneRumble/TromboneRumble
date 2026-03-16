// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PartyWidget.generated.h"

class UPartyPlayerWidget;
class UDynamicEntryBox;
class AEasyPartyPlayerState;

UCLASS()
class TROMBONERUMBLE_API UPartyWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
	public:

	/** Default constructor. */
	UPartyWidget();

public:
	/** Whether a player widget should be created for the local player or not. */
	UPROPERTY(EditDefaultsOnly)
	bool bCreateEntryForLocalPlayer;
	
	/** Player widget class to use when creating player widgets for party members. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPartyPlayerWidget> PartyPlayerWidgetClass;

protected:

	/** Dynamic entry box that handles player widgets. */
	UPROPERTY(meta = (BindWidget))
	UDynamicEntryBox* PartyPlayerEntryBox;

public:

	/** Creates a player widget for the given player. */
	virtual void CreatePlayerWidget(AEasyPartyPlayerState* OwningPlayerState);

	/** Removes the player widget of an existing player. */
	virtual void RemovePlayerWidget(const FUniqueNetIdRepl& PlayerId);

protected:
	/** Called when we left a party. */
	UFUNCTION()
	virtual void OnDisconnectedFromParty();

	/** Called when a new player state is added to the party. */
	UFUNCTION()
	virtual void OnPlayerStateAdded(AEasyPartyPlayerState* PlayerState);

	/** Called when an existing player state is being removed from the party. */
	UFUNCTION()
	virtual void OnPlayerStateRemoved(AEasyPartyPlayerState* PlayerState);

public:
	
	//~ Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface
	
};
