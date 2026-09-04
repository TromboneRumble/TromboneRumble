// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "Utilities/Defines.h"
#include "MatchMenuRows.generated.h"

/** One entry of the match type rotator. Row order is the display order. */
USTRUCT()
struct FMatchTypeRow : public FTableRowBase
{
	GENERATED_BODY()
	
	/** The match type this entry stands for. */
	UPROPERTY(EditAnywhere)
	EMatchType Type = EMatchType::Public;
	
	/** Name shown on the rotator. */
	UPROPERTY(EditAnywhere)
	FText Label;
	
	/** Shown behind the rotator while this entry is selected. Leave empty for no background. */
	UPROPERTY(EditAnywhere)
	FSlateBrush Background;
};

/** One entry of the map rotator. Row order is the display order.
 *  GameMapDeveloperSettings decides which maps exist and which are hidden in shipping.
 *  This table only holds how each map is shown. */
USTRUCT()
struct FSelectableMapRow : public FTableRowBase
{
	GENERATED_BODY()
	
	/** Traveling to this stage actually goes to its lobby map. */
	UPROPERTY(EditAnywhere, meta = (Categories = "Trombone.Maps.Lobby"))
	FGameplayTag LobbyMapTag;
	
	/** Name shown on the rotator. */
	UPROPERTY(EditAnywhere)
	FText Label;
	
	/** Shown behind the rotator while this entry is selected. Leave empty for no background. */
	UPROPERTY(EditAnywhere)
	FSlateBrush Background;
};

/** Turns the raw tables into the final lists the match menu shows. Validation and logging live here. */
namespace MatchMenuOptions
{
	struct FMapEntry
	{
		FGameplayTag Tag;
		FText Label;
		FSlateBrush Background;
	};
	
	struct FMatchTypeEntry
	{
		EMatchType Type;
		FText Label;
		FSlateBrush Background;
	};
	
	/** Table order decides display order. GameMapDeveloperSettings decides which maps may appear. */
	TArray<FMapEntry> BuildSelectableMaps(const UDataTable* Table);
	
	/** Table order decides display order. An empty result means the table is missing or empty. */
	TArray<FMatchTypeEntry> BuildMatchTypes(const UDataTable* Table);
}
