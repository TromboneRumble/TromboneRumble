// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/MatchMenuRows.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"

namespace
{
	/** "Trombone.Maps.Lobby.OrchestraStage" -> "OrchestraStage" */
	FString GetTagLeaf(const FGameplayTag& Tag)
	{
		const FString TagStr = Tag.ToString();
		FString Leaf;
		if (!TagStr.Split(TEXT("."), nullptr, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			Leaf = TagStr;
		}
		return Leaf;
	}
}

TArray<MatchMenuOptions::FMapEntry> MatchMenuOptions::BuildSelectableMaps(const UDataTable* Table)
{
	TArray<FMapEntry> Entries;

	// GameMapDeveloperSettings decides which maps exist. The table only picks how they look and their order
	const FGameplayTag LobbyCategory = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::LobbyPath), false);
	const TArray<FGameplayTag> AllowedTags = UTromboneFunctionLibrary::GetMapTagsUnderCategory(LobbyCategory);

	auto HasTag = [&Entries](const FGameplayTag& Tag)
	{
		return Entries.ContainsByPredicate([&Tag](const FMapEntry& Entry) { return Entry.Tag == Tag; });
	};

	if (Table)
	{
		// Table order, allowed maps only
		Table->ForeachRow<FSelectableMapRow>(TEXT("BuildSelectableMaps"),
			[&](const FName& RowName, const FSelectableMapRow& Row)
			{
				if (HasTag(Row.LobbyMapTag))
				{
					UE_LOG(LogTemp, Warning, TEXT("[MatchMenu] SelectableMapTable %s: duplicate tag %s, row skipped"),
						*RowName.ToString(), *Row.LobbyMapTag.ToString());
					return;
				}
				if (!AllowedTags.Contains(Row.LobbyMapTag))
				{
					// Normal for maps hidden in shipping. A typo in the tag also lands here
					UE_LOG(LogTemp, Log, TEXT("[MatchMenu] SelectableMapTable %s: %s is not selectable, row skipped"),
						*RowName.ToString(), *Row.LobbyMapTag.ToString());
					return;
				}

				Entries.Add({ Row.LobbyMapTag, Row.Label, Row.Background });
			});
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MatchMenu] SelectableMapTable is not set. Maps get placeholder names"));
	}

	// A selectable map without a row still shows up, under its tag leaf name
	for (const FGameplayTag& Tag : AllowedTags)
	{
		if (HasTag(Tag))
		{
			continue;
		}
		UE_LOG(LogTemp, Warning, TEXT("[MatchMenu] SelectableMapTable has no row for %s. Shown with a placeholder name"), *Tag.ToString());
		Entries.Add({ Tag, FText::FromString(GetTagLeaf(Tag)), FSlateBrush() });
	}

	return Entries;
}

TArray<MatchMenuOptions::FMatchTypeEntry> MatchMenuOptions::BuildMatchTypes(const UDataTable* Table)
{
	TArray<FMatchTypeEntry> Entries;

	if (!Table)
	{
		UE_LOG(LogTemp, Error, TEXT("[MatchMenu] MatchTypeTable is not set. The match type rotator stays empty"));
		return Entries;
	}

	Table->ForeachRow<FMatchTypeRow>(TEXT("BuildMatchTypes"),
		[&Entries](const FName& RowName, const FMatchTypeRow& Row)
		{
			const bool bDuplicate = Entries.ContainsByPredicate(
				[&Row](const FMatchTypeEntry& Entry) { return Entry.Type == Row.Type; });
			if (bDuplicate)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MatchMenu] MatchTypeTable %s: duplicate type, row skipped"), *RowName.ToString());
				return;
			}

			Entries.Add({ Row.Type, Row.Label, Row.Background });
		});

	if (Entries.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[MatchMenu] MatchTypeTable is empty. The match type rotator stays empty"));
	}

	return Entries;
}
