// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/** When fever time starts and when the round ends in one song, counted from the Event_Spotlight_Start cue like the timeline. */
struct FGimmickSongTiming
{
	/** Row tag of the song in the song table. */
	FGameplayTag SongTag;

	/** Song name from the song table. */
	FString SongName;

	/** Seconds from the start cue to the fever cue. */
	float FeverStart = 0.f;

	/** Seconds from the start cue to the end of the BGM event, where the round ends. */
	float RoundLength = 0.f;
};

/**
 * Reads the song timings from the Wwise project each time instead of storing them, so a cue moved in Wwise shows up on the next read.
 * The song table gives the BGM event of each song, and the events work unit gives the music segment the event plays.
 * The music work unit gives the cue times of that segment, and SoundbanksInfo.json gives the length of the event.
 */
namespace GimmickSongTiming
{
	/**
	 * Read every song that has both cues and a known length.
	 *
	 * @param OutSkipped One line per song that could not be read, with the reason.
	 * @return The songs in the order of the song table.
	 */
	TArray<FGimmickSongTiming> CollectSongTimings(TArray<FString>& OutSkipped);

	/** @return Songs the open level can get, or none when the open level is not an in-game map. */
	TArray<FGameplayTag> FindSongsOfLevel(const UWorld* World);
}
