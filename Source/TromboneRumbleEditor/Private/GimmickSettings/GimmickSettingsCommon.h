// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IDetailsView;
class UWorld;

/** Helpers the widgets of the gimmick settings panel share. */
namespace GimmickSettings
{
	/** Left indent of the rows under a section header, so a header and its rows do not look alike. */
	constexpr float SectionRowIndent = 16.f;

	/** @return World of the level open in the editor, or null. */
	UWorld* GetOpenLevelWorld();

	/** @return Whether a PIE session runs. */
	bool IsPlayingInEditor();

	/**
	 * Make a details view for the panel, without the name area, the search box and the selection lock.
	 *
	 * @param bManyObjects Shows each object as its own section instead of merging them.
	 */
	TSharedRef<IDetailsView> CreateDetailsView(bool bManyObjects);
}
