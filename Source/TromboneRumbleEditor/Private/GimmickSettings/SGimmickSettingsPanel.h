// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "EditorUndoClient.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class AGimmickManager;
class IDetailsView;
class SGimmickIssueList;
class SGimmickPlayControls;
class SGimmickTimelineTab;
struct FAssetData;
struct FPropertyChangedEvent;

/**
 * SGimmickSettingsPanel is the one place a designer edits gimmick settings.
 * It is the content of the "기믹 설정" tab, created by FTromboneRumbleEditorModule.
 *
 * Top: pick a UStageGimmickData asset and save everything that changed, then the status line and SGimmickIssueList.
 * "레벨 설정" tab: the asset in a details view. Entries are added, removed and edited here. These settings belong to one level.
 * "타임라인" tab: SGimmickTimelineTab.
 * Bottom: SGimmickPlayControls.
 *
 * The panel owns the asset and decides when the sections rebuild. The sections do not watch the editor themselves.
 * Every change ends in RebuildSections, so that one function shows what the panel draws again.
 *
 * The current level is the level PIE plays while PIE runs, so a level change inside PIE moves the panel along.
 *
 * Keep in mind that it refreshes itself when the current level changes, when a gimmick actor is placed or removed,
 * when the manager gets another asset, when the asset changes in any editor, and on undo and redo. Moving an actor does not refresh it.
 *
 * @see UStageGimmickData
 * @see AGimmickBase
 */
class SGimmickSettingsPanel : public SCompoundWidget, public FSelfRegisteringEditorUndoClient
{
public:

	SLATE_BEGIN_ARGS(SGimmickSettingsPanel) {}
	SLATE_END_ARGS()

	/** Build the widget. Starts on the stage data of the current level. */
	void Construct(const FArguments& InArgs);

	/** Unbind from the editor delegates. */
	virtual ~SGimmickSettingsPanel() override;

	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient Interface

private:

	/**
	 * Refresh once on the next frame, however many changes ask for it before then.
	 *
	 * @param bLevelChanged The current level changed, so the songs of the timeline are read again too.
	 */
	void RequestRefresh(bool bLevelChanged = false);

	/** Run the refresh RequestRefresh asked for. */
	EActiveTimerReturnType HandleRefreshTimer(double InCurrentTime, float InDeltaTime);

	/**
	 * Follow a new asset of the manager of the current level, then rebuild the sections and the status line.
	 * The panel follows only while it shows the asset of the current level, so an asset the designer picked by hand stays.
	 */
	void Refresh();

	/** Rebuild the problem list, the timeline tab and the PIE rows from the asset and the current level. */
	void RebuildSections();

	/** Make the status line again. It changes only with the asset or the manager, so it is not made every frame. */
	void UpdateStatusText();

	/** A gimmick actor or the manager was placed in or removed from the current level. */
	void HandleLevelActorChanged(AActor* Actor);

	/** Any object changed in any editor. Only the asset, the manager and the gimmick actors matter. */
	void HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);

	/** A map was opened in the editor. */
	void HandleMapOpened(const FString& Filename, bool bAsTemplate);

	/** A level was loaded, by a level change inside PIE among others. Only a PIE world matters. */
	void HandleLevelLoaded(UWorld* LoadedWorld);

	/** PIE started or ended, so the current level moved between the editor world and the PIE world. */
	void HandlePlayStartedOrEnded(bool bIsSimulating);

	/** Show this asset in the details view, then rebuild the sections and the status line. */
	void SetStageData(UStageGimmickData* NewStageData);

	/** Switch to the asset the gimmick manager of the current level points at. */
	void UseStageDataOfCurrentLevel();

	void HandleAssetPicked(const FAssetData& AssetData);

	/** @return Whether the asset has changes that are not saved. */
	bool HasUnsavedChanges() const;

	FReply HandleSaveClicked();

	/** Asset picker, the button that goes back to the asset of the current level, and the save button. */
	TSharedRef<SWidget> MakeTopBar();

	/** Status line and the problem list. */
	TSharedRef<SWidget> MakeStatusArea();

	/** Buttons that switch between the two tabs. */
	TSharedRef<SWidget> MakeTabBar();

	/** The two tabs. Only the one picked in the tab bar shows. */
	TSharedRef<SWidget> MakeTabContent();

	/** One line that says which level is open and which asset its manager uses. */
	FText GetStatusText() const { return StatusText; }

	/** Collapsed while no stage data asset is selected. Bound to everything that shows the asset or what it lists. */
	EVisibility GetStageDataAreaVisibility() const;

	/** @return The first gimmick manager of the current level that has stage data, or null. */
	static AGimmickManager* FindManagerOfCurrentLevel();

	/** @return Asset the manager of the current level points at, or null. */
	static UStageGimmickData* FindStageDataOfCurrentLevel();

	/** Asset being edited. Strong, because nothing else may reference an asset picked by hand. */
	TStrongObjectPtr<UStageGimmickData> StageData;

	/** Asset the manager of the current level pointed at when the panel last looked. */
	TWeakObjectPtr<UStageGimmickData> CurrentLevelStageData;

	/** Made by UpdateStatusText. */
	FText StatusText;

	/** Has a refresh been asked for and not run yet. */
	bool bRefreshPending = false;

	/** Did the current level change since the last refresh. */
	bool bLevelChangePending = false;

	/** 0 is the level tab, 1 is the timeline tab. */
	int32 ActiveTabIndex = 0;

	/** Details view of StageData, the content of the level tab. */
	TSharedPtr<IDetailsView> StageDetailsView;

	TSharedPtr<SGimmickIssueList> IssueList;

	TSharedPtr<SGimmickTimelineTab> TimelineTab;

	TSharedPtr<SGimmickPlayControls> PlayControls;

	FDelegateHandle MapOpenedHandle;

	FDelegateHandle LevelLoadedHandle;

	FDelegateHandle PlayStartedHandle;

	FDelegateHandle PlayEndedHandle;

	FDelegateHandle ActorAddedHandle;

	FDelegateHandle ActorDeletedHandle;

	FDelegateHandle ObjectPropertyChangedHandle;
};
