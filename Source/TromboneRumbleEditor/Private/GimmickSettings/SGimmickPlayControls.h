// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Defines.h"
#include "Widgets/SCompoundWidget.h"

class AGimmickBase;
class SVerticalBox;
class UStageGimmickData;

/**
 * SGimmickPlayControls is the bottom of the gimmick settings panel.
 * First comes one row per gimmick with buttons that run the gimmick console commands in PIE.
 * Then come the gimmicks of the open level that the panel cannot edit, each with a button that selects it in the level.
 * A section with no row is hidden.
 *
 * Keep in mind that it does not watch anything. SGimmickSettingsPanel calls Rebuild when the asset or the level changes.
 *
 * @see SGimmickSettingsPanel
 */
class SGimmickPlayControls : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickPlayControls) {}
	SLATE_END_ARGS()

	/** Build the two empty sections. */
	void Construct(const FArguments& InArgs);

	/**
	 * Make the rows again.
	 *
	 * @param World Open level. Its gimmick actors get rows.
	 * @param StageData Asset shown in the panel. Its entries decide which gimmicks the panel can edit,
	 *        and an entry with a Blueprint for the manager to spawn gets a row too.
	 */
	void Rebuild(const UWorld* World, const UStageGimmickData* StageData);

private:

	/** A label and the Trigger, Stop and Restart buttons for one gimmick type. */
	static TSharedRef<SWidget> MakeGimmickRow(const FText& Label, EGimmickType GimmickType);

	/** Label of a level gimmick the panel cannot edit and a button that selects it in the level. */
	static TSharedRef<SWidget> MakeUnmanagedRow(AGimmickBase& Gimmick);

	/** Run a gimmick console command in every PIE world that is not a client. */
	static void RunGimmickCommand(const TCHAR* Command, EGimmickType GimmickType);

	/** One row per gimmick, with the PIE buttons. */
	TSharedPtr<SVerticalBox> GimmickRows;

	/** One row per gimmick the panel cannot edit. */
	TSharedPtr<SVerticalBox> UnmanagedRows;
};
