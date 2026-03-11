// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PartyPlayerWidget.generated.h"

class UCommonTextBlock;
class AEasyPartyPlayerState;

UCLASS()
class TROMBONERUMBLE_API UPartyPlayerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UPartyPlayerWidget();

protected:

	/** Player state of the owning player. */
	UPROPERTY(Transient)
	AEasyPartyPlayerState* OwningPlayerState;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Leader;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Name;

public:

	/** Initializes the player widget. */
	virtual void InitPlayerWidget(AEasyPartyPlayerState* InOwningPlayerState);

	/** Get the owning player's unique id. */
	UFUNCTION(BlueprintPure, Category = "Default")
	FUniqueNetIdRepl& GetPlayerUniqueId() const;

	/** Get the owning player's name. */
	UFUNCTION(BlueprintPure, Category = "Default")
	FText GetPlayerName() const;

	/** Whether the owning player is the party leader or not. */
	UFUNCTION(BlueprintPure, Category = "Default")
	bool IsPartyLeader() const;

	/** Get the owning player's party player state. */
	UFUNCTION(BlueprintPure, Category = "Default")
	AEasyPartyPlayerState* GetOwningPartyPlayerState() const { return OwningPlayerState; }

protected:
	/** Called when the party leader changes (including the first time it replicates). */
	virtual void OnPartyLeaderChanged(const FUniqueNetIdRepl& UniqueId);
	
};
