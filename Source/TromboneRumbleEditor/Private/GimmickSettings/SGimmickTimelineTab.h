// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GimmickSettings/GimmickSongTiming.h"
#include "Widgets/SCompoundWidget.h"

class SGimmickTimeline;
class UStageGimmickData;
template <typename OptionType> class SComboBox;

/**
 * SGimmickTimelineTab is the "타임라인" tab of the gimmick settings panel.
 * It shows when each gimmick of the asset is expected to run during one round, and it is read only.
 * The round length and the fever start come from the song picked in the song list, or the designer types them.
 *
 * Keep in mind that it does not watch the asset. SGimmickSettingsPanel calls Rebuild when the asset changes, and RefreshSongs when a map opens.
 *
 * @see SGimmickSettingsPanel
 * @see SGimmickTimeline
 */
class SGimmickTimelineTab : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickTimelineTab) {}
	SLATE_END_ARGS()

	/** Build the tab and read the songs. */
	void Construct(const FArguments& InArgs);

	/**
	 * Draw the timeline of this asset again.
	 *
	 * @param InStageData Asset to draw. Null draws no row.
	 */
	void Rebuild(const UStageGimmickData* InStageData);

	/** Read the songs again and pick the one remembered for the current level, or else the first song in the list. */
	void RefreshSongs();

private:

	/** Ask the asset for its timeline rows with the current length, fever start and seed, and give them to the timeline. */
	void RebuildRows();

	/** Use the timing of this song on the timeline. The entry without a song tag switches to typing the values by hand. */
	void SelectSong(TSharedPtr<FGimmickSongTiming> Song);

	/** @return Label of an entry of the song list. */
	FText GetSongLabel(const TSharedPtr<FGimmickSongTiming>& Song) const;

	/** @return Key of the current level in SongPerLevel. */
	static FString GetCurrentLevelKey();

	/** Visible only while the timeline values are typed by hand. Bound to the round length and fever start boxes. */
	EVisibility GetManualTimingVisibility() const;

	/** Asset drawn. */
	TWeakObjectPtr<const UStageGimmickData> StageData;

	/** Draws the rows. */
	TSharedPtr<SGimmickTimeline> Timeline;

	/** The song list. */
	TSharedPtr<SComboBox<TSharedPtr<FGimmickSongTiming>>> SongComboBox;

	/** Seconds the timeline covers. The picked song fills it, or the designer types it. */
	float TimelineRoundLength = 210.f;

	/** Second of the timeline the fever cue of the song comes at. The picked song fills it, or the designer types it. */
	float TimelineFeverStart = 168.f;

	/** Entries of the song list. The songs of the current level come first, and the entry without a song tag at the end is for typing by hand. */
	TArray<TSharedPtr<FGimmickSongTiming>> SongOptions;

	/** Entry of the song list the timeline uses. */
	TSharedPtr<FGimmickSongTiming> SelectedSong;

	/** Songs the current level can get. Their labels say so. */
	TArray<FGameplayTag> LevelSongs;

	/** Why songs are missing from the list, one line each. Shown in the tooltip of the song list. */
	FString SkippedSongs;

	/** Song picked last in each level, by level package. An invalid tag means typing by hand. Kept while the editor runs. */
	TMap<FString, FGameplayTag> SongPerLevel;

	/** Seed of the random waits on the timeline. The same seed draws the same timeline, so an edit does not shuffle it. */
	int32 TimelineSeed = 1;
};
