// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AGimmickManager;
class SVerticalBox;
class UStageGimmickData;

/**
 * SGimmickIssueList lists the problems of a stage data asset above the tabs of the gimmick settings panel.
 * It shows "문제 없음" when there is none, and otherwise an area with one row per error or warning.
 *
 * Keep in mind that it does not watch anything. SGimmickSettingsPanel calls Rebuild when the asset or the level changes.
 *
 * @see SGimmickSettingsPanel
 */
class SGimmickIssueList : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGimmickIssueList) {}
	SLATE_END_ARGS()

	/** Build the empty list. */
	void Construct(const FArguments& InArgs);

	/**
	 * List the problems again.
	 *
	 * @param StageData Asset to check. Null clears the list.
	 * @param LevelManager Manager of the open level, only when the open level uses this asset. Its level checks are added then.
	 */
	void Rebuild(const UStageGimmickData* StageData, const AGimmickManager* LevelManager);

private:

	/** @return Title of the area, with the count of errors and warnings. */
	FText GetTitle() const;

	/** One row per problem. */
	TSharedPtr<SVerticalBox> Rows;

	/** Counted by Rebuild for the title. */
	int32 NumErrors = 0;

	int32 NumWarnings = 0;
};
