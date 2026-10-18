// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "GimmickSettings/GimmickSongTiming.h"
#include "UObject/StrongObjectPtr.h"
#include "Utilities/Defines.h"
#include "Widgets/SCompoundWidget.h"

class AGimmickBase;
class IDetailsView;
class SGimmickTimeline;
class SVerticalBox;
class UGimmickConfig;
struct FAssetData;
struct FPropertyChangedEvent;
template <typename OptionType> class SComboBox;

/**
 * SGimmickSettingsPanel is the one place a designer edits gimmick settings.
 * It is the content of the "기믹 설정" tab, created by FTromboneRumbleEditorModule.
 *
 * Top: pick a UStageGimmickData asset and save everything that changed.
 * "레벨 설정" tab: the asset in a details view. Entries are added, removed and edited here. These settings belong to one level.
 * "투척물 종류별 수치" tab: objects the gimmicks of the open level report through AGimmickBase::GetSettingsObjects.
 * Right now these are the garbage Blueprints, whose knockback and hit type differ per kind of garbage.
 * "타임라인" tab: when each gimmick of the asset is expected to run during one round. Read only.
 * Bottom: one row per gimmick of the open level with buttons that run the gimmick console commands in PIE,
 * then the gimmicks of the open level that this panel cannot edit at all.
 *
 * @see UStageGimmickData
 * @see AGimmickBase
 */
class SGimmickSettingsPanel : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickSettingsPanel) {}
	SLATE_END_ARGS()

	/** Build the widget. Starts on the stage data of the open level. */
	void Construct(const FArguments& InArgs);

	/** Unbind from the editor delegates. */
	virtual ~SGimmickSettingsPanel() override;

private:

	/** Show this asset in the details view and rebuild the lists. */
	void SetStageData(UStageGimmickData* NewStageData);

	/** Switch to the asset the gimmick manager of the open level points at. */
	void UseStageDataOfOpenLevel();

	/** Rebuild the shared data tab, the PIE rows and the list of gimmicks this panel cannot edit. */
	void RebuildLists();

	/** Ask the asset for its timeline rows and give them to the timeline. */
	void RebuildTimeline();

	/** Check the asset, and the level too when the open level uses it, and list the problems above the tabs. */
	void RebuildIssues();

	/** @return Title of the problem list, with the count of errors and warnings. */
	FText GetIssuesTitle() const;

	/** Song list, round length, fever start, the button that picks new random waits, the legend and the timeline. */
	TSharedRef<SWidget> MakeTimelineTab();

	/** Read the songs again and pick the one remembered for the open level, or else the first song in the list. */
	void RefreshSongs();

	/** Use the timing of this song on the timeline. The entry without a song tag switches to typing the values by hand. */
	void SelectSong(TSharedPtr<FGimmickSongTiming> Song);

	/** @return Label of an entry of the song list. */
	FText GetSongLabel(const TSharedPtr<FGimmickSongTiming>& Song) const;

	/** @return Key of the open level in SongPerLevel. */
	static FString GetOpenLevelKey();

	/** A label and the Trigger, Stop and Restart buttons for one gimmick type. */
	TSharedRef<SWidget> MakeGimmickRow(const FText& Label, EGimmickType GimmickType);

	/** Label of a level gimmick this panel cannot edit and a button that selects it in the level. */
	TSharedRef<SWidget> MakeUnmanagedRow(AGimmickBase& Gimmick);

	/** Add the category path of every GimmickSetting property of this class, and each shorter path above it. */
	void CollectSettingCategories(const UClass& Class);

	/** Bound to the second tab. True when a custom row or a subcategory sits in a category that holds gimmick settings. */
	bool IsSettingCategory(FName RowName, FName ParentCategory) const;

	/** One line that says which level is open and which asset its manager uses. */
	FText GetStatusText() const;

	/** Collapsed while no stage data asset is selected. Bound to the tab bar, its separator and the two details views. */
	EVisibility GetStageDataAreaVisibility() const;

	/** Visible only while the timeline values are typed by hand. Bound to the round length and fever start boxes. */
	EVisibility GetManualTimingVisibility() const;

	/** @return Packages of the stage data and of every shared object. */
	TArray<UPackage*> GetEditedPackages() const;

	bool HasUnsavedChanges() const;

	FReply HandleSaveClicked();

	void HandleAssetPicked(const FAssetData& AssetData);

	void HandleMapOpened(const FString& Filename, bool bAsTemplate);

	/** An entry was added, removed or edited in the details view. */
	void HandlePropertiesChanged(const FPropertyChangedEvent& Event);

	/** Run a gimmick console command in every PIE world that is not a client. */
	static void RunGimmickCommand(const TCHAR* Command, EGimmickType GimmickType);

	/** @return Asset the manager of the open level points at, or null. */
	static UStageGimmickData* FindStageDataOfOpenLevel();

	/** Asset being edited. Strong, because nothing else may reference an asset picked by hand. */
	TStrongObjectPtr<UStageGimmickData> StageData;

	/** Details view of StageData. */
	TSharedPtr<IDetailsView> StageDetailsView;

	/** Objects shown in the shared data tab. Filled by RebuildLists. */
	TArray<TWeakObjectPtr<UObject>> SharedObjects;

	/** Details view of SharedObjects. Each object is one section with its own header. */
	TSharedPtr<IDetailsView> SharedDetailsView;

	/** Categories that hold GimmickSetting properties of the shown objects. Filled by RebuildLists before the layout is built. */
	TSet<FName> SettingCategories;

	/** Filled by RebuildLists. */
	TSharedPtr<SVerticalBox> GimmickRows;

	/** Filled by RebuildLists. */
	TSharedPtr<SVerticalBox> UnmanagedRows;

	/** Filled by RebuildTimeline. */
	TSharedPtr<SGimmickTimeline> Timeline;

	/** One row per problem. Filled by RebuildIssues. */
	TSharedPtr<SVerticalBox> IssueRows;

	/** Counted by RebuildIssues for the title of the list. */
	int32 NumIssueErrors = 0;

	int32 NumIssueWarnings = 0;

	/** Seconds the timeline covers. The picked song fills it, or the designer types it. */
	float TimelineRoundLength = 210.f;

	/** Second of the timeline the fever cue of the song comes at. The picked song fills it, or the designer types it. */
	float TimelineFeverStart = 168.f;

	/** Entries of the song list. The songs of the open level come first, and the entry without a song tag at the end is for typing by hand. */
	TArray<TSharedPtr<FGimmickSongTiming>> SongOptions;

	/** Entry of the song list the timeline uses. */
	TSharedPtr<FGimmickSongTiming> SelectedSong;

	/** Songs the open level can get. Their labels say so. */
	TArray<FGameplayTag> LevelSongs;

	/** Why songs are missing from the list, one line each. Shown in the tooltip of the song list. */
	FString SkippedSongs;

	/** Song picked last in each level, by level package. An invalid tag means typing by hand. Kept while the editor runs. */
	TMap<FString, FGameplayTag> SongPerLevel;

	TSharedPtr<SComboBox<TSharedPtr<FGimmickSongTiming>>> SongComboBox;

	/** Seed of the random waits on the timeline. The same seed draws the same timeline, so an edit does not shuffle it. */
	int32 TimelineSeed = 1;

	/** 0 is the level tab, 1 is the shared data tab, 2 is the timeline tab. */
	int32 ActiveTabIndex = 0;

	FDelegateHandle MapOpenedHandle;
};
