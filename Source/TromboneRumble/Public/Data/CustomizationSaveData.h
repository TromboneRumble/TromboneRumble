// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomizationSaveData.generated.h"

/**
 * Stores the player's current customization selections.
 * Each field holds the RowName (Key) of the selected FCustomizationPartRow.
 * NAME_None means no saved selection — the component will fall back to AssetOrder 0.
 */
USTRUCT(BlueprintType)
struct FCustomizationSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName AntennaKey = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FName FaceKey = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FName CostumeKey = NAME_None;

	bool operator==(const FCustomizationSaveData& Other) const
	{
		return AntennaKey == Other.AntennaKey
			&& FaceKey    == Other.FaceKey
			&& CostumeKey == Other.CostumeKey;
	}

	bool operator!=(const FCustomizationSaveData& Other) const
	{
		return !(*this == Other);
	}
};
