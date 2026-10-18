// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickIssueList.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "Misc/DataValidation.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GimmickSettings"

void SGimmickIssueList::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoIssues", "문제 없음"))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.Visibility_Lambda([this]() { return NumErrors + NumWarnings == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SExpandableArea)
			.Visibility_Lambda([this]() { return NumErrors + NumWarnings > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
			.AreaTitle(this, &SGimmickIssueList::GetTitle)
			.InitiallyCollapsed(false)
			.BodyContent()
			[
				SAssignNew(Rows, SVerticalBox)
			]
		]
	];
}

void SGimmickIssueList::Rebuild(const UStageGimmickData* StageData, const AGimmickManager* LevelManager)
{
	Rows->ClearChildren();
	NumErrors = 0;
	NumWarnings = 0;
	if (!StageData) return;

	TArray<FDataValidationContext::FIssue> Issues;
	{
		FDataValidationContext Context;
		StageData->IsDataValid(Context);
		Issues = Context.GetIssues();
	}

	if (LevelManager)
	{
		TArray<FText> LevelIssues;
		LevelManager->CollectLevelIssues(LevelIssues);
		for (const FText& Issue : LevelIssues)
		{
			Issues.Emplace(Issue, EMessageSeverity::Warning);
		}
	}

	for (const FDataValidationContext::FIssue& Issue : Issues)
	{
		const bool bError = Issue.Severity == EMessageSeverity::Error;
		++(bError ? NumErrors : NumWarnings);

		Rows->AddSlot().AutoHeight().Padding(GimmickSettings::SectionRowIndent, 2.f, 0.f, 2.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 1.f, 6.f, 0.f)
			[
				SNew(SImage).Image(FAppStyle::GetBrush(bError ? "Icons.ErrorWithColor" : "Icons.WarningWithColor"))
			]

			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock).Text(Issue.Message).AutoWrapText(true)
			]
		];
	}
}

FText SGimmickIssueList::GetTitle() const
{
	return FText::Format(LOCTEXT("IssuesTitle", "문제 {0}개 (오류 {1}, 경고 {2})"), NumErrors + NumWarnings, NumErrors, NumWarnings);
}

#undef LOCTEXT_NAMESPACE
