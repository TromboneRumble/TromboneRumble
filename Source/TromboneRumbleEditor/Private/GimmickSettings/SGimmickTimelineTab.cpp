// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickTimelineTab.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "GimmickSettings/SGimmickTimeline.h"
#include "Data/Gimmick/GimmickTimeline.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GimmickSettings"

using GimmickSettings::GetOpenLevelWorld;

namespace
{
	/** @return Whether the entry of the song list is a song, and not the entry for typing by hand. */
	bool IsSong(const TSharedPtr<FGimmickSongTiming>& Entry)
	{
		return Entry.IsValid() && Entry->SongTag.IsValid();
	}
}

void SGimmickTimelineTab::Construct(const FArguments& InArgs)
{
	const auto MakeLegendEntry = [](const EGimmickTimelinePhase Phase, const FText& Text)
	{
		return SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(12.f).HeightOverride(12.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(SGimmickTimeline::GetPhaseColor(Phase))
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 16.f, 0.f)
			[
				SNew(STextBlock).Text(Text)
			];
	};

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("Song", "곡"))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(SongComboBox, SComboBox<TSharedPtr<FGimmickSongTiming>>)
				.OptionsSource(&SongOptions)
				.ToolTipText_Lambda([this]()
				{
					FString Text = TEXT("곡을 고르면 Wwise 프로젝트에서 그 곡의 피버 큐와 길이를 읽어 타임라인에 씁니다. 이 레벨에서 나오는 곡이 먼저 나옵니다. 직접 입력을 고르면 두 값을 손으로 넣습니다");
					if (!SkippedSongs.IsEmpty())
					{
						Text += TEXT("\n\n목록에 없는 곡:\n") + SkippedSongs;
					}
					return FText::FromString(Text);
				})
				.OnGenerateWidget_Lambda([this](const TSharedPtr<FGimmickSongTiming> Song) -> TSharedRef<SWidget> { return SNew(STextBlock).Text(GetSongLabel(Song)); })
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FGimmickSongTiming> Song, const ESelectInfo::Type SelectInfo)
				{
					// RefreshSongs sets the starting song itself, and that is not a choice of the designer
					if (SelectInfo == ESelectInfo::Direct) return;

					SongPerLevel.Add(GetOpenLevelKey(), Song.IsValid() ? Song->SongTag : FGameplayTag());
					SelectSong(Song);
				})
				[
					SNew(STextBlock).Text_Lambda([this]() { return GetSongLabel(SelectedSong); })
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([this]() { return IsSong(SelectedSong) ? EVisibility::Visible : EVisibility::Collapsed; })
				.Text_Lambda([this]()
				{
					return IsSong(SelectedSong)
						? FText::Format(LOCTEXT("SongTiming", "피버 {0} · 길이 {1}"), SGimmickTimeline::FormatTime(SelectedSong->FeverStart), SGimmickTimeline::FormatTime(SelectedSong->RoundLength))
						: FText::GetEmpty();
				})
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility(this, &SGimmickTimelineTab::GetManualTimingVisibility)
				.Text(LOCTEXT("RoundLength", "라운드 길이"))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(80.f)
				.Visibility(this, &SGimmickTimelineTab::GetManualTimingVisibility)
				[
					SNew(SSpinBox<float>)
					.MinValue(10.f)
					.MaxValue(600.f)
					.Delta(1.f)
					.ToolTipText(LOCTEXT("RoundLengthTip", "타임라인이 보여줄 길이(초)입니다. 기믹이 켜지는 시점부터 곡이 끝날 때까지의 시간을 넣습니다. 이 창에서만 쓰고 저장되지 않습니다"))
					.Value_Lambda([this]() { return TimelineRoundLength; })
					.OnValueChanged_Lambda([this](const float NewValue) { TimelineRoundLength = NewValue; RebuildRows(); })
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility(this, &SGimmickTimelineTab::GetManualTimingVisibility)
				.Text(LOCTEXT("FeverStart", "피버 시작"))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(80.f)
				.Visibility(this, &SGimmickTimelineTab::GetManualTimingVisibility)
				[
					SNew(SSpinBox<float>)
					.MinValue(0.f)
					.MaxValue(600.f)
					.Delta(1.f)
					.ToolTipText(LOCTEXT("FeverStartTip", "곡의 피버 큐(Event_Spotlight_Fever)가 오는 시점(초)입니다. 피버 타임 시 강화를 켠 스포너는 이 시점부터 피버 스폰 규칙을 쓰고, 피버 타임 시 순서 변경을 켠 기믹 시퀀스는 이 시점 이후 차례부터 피버 타임 시 시퀀스 순서를 씁니다. 이 창에서만 쓰고 저장되지 않습니다"))
					.Value_Lambda([this]() { return TimelineFeverStart; })
					.OnValueChanged_Lambda([this](const float NewValue) { TimelineFeverStart = NewValue; RebuildRows(); })
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Reroll", "다시 뽑기"))
				.ToolTipText(LOCTEXT("RerollTip", "스폰 간격이 최소~최대인 기믹은 실제 게임에서 매번 다르게 나옵니다. 타임라인은 그중 한 가지 경우를 보여주며, 이 버튼으로 다른 경우를 봅니다. 간격이 고정이거나 기믹 시퀀스로 도는 기믹은 바뀌지 않습니다"))
				.OnClicked_Lambda([this]() { ++TimelineSeed; RebuildRows(); return FReply::Handled(); })
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeLegendEntry(EGimmickTimelinePhase::Warning, LOCTEXT("LegendWarning", "예고"))
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeLegendEntry(EGimmickTimelinePhase::Active, LOCTEXT("LegendActive", "발동 중"))
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeLegendEntry(EGimmickTimelinePhase::Ending, LOCTEXT("LegendEnding", "사라지는 중"))
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeLegendEntry(EGimmickTimelinePhase::Spawn, LOCTEXT("LegendSpawn", "스폰 시점"))
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TimelineNote", "0:00은 라운드 시작이 아니라 곡의 Event_Spotlight_Start 큐에서 기믹이 켜지는 시점입니다. 블록에 마우스를 올리면 단계와 시간이 보이고, 기믹 이름에 올리면 그 줄을 읽을 때 주의할 점이 보입니다."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot().FillHeight(1.f).Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			[
				SAssignNew(Timeline, SGimmickTimeline)
			]
		]
	];

	RefreshSongs();
}

void SGimmickTimelineTab::Rebuild(const UStageGimmickData* InStageData)
{
	StageData = InStageData;
	RebuildRows();
}

void SGimmickTimelineTab::RebuildRows()
{
	if (!Timeline.IsValid()) return;

	TArray<FGimmickTimelineRow> Rows;
	if (StageData.IsValid())
	{
		Rows = StageData->BuildTimeline(TimelineRoundLength, TimelineFeverStart, TimelineSeed, GetOpenLevelWorld());
	}
	Timeline->SetRows(MoveTemp(Rows), TimelineRoundLength, TimelineFeverStart);
}

void SGimmickTimelineTab::RefreshSongs()
{
	TArray<FString> Skipped;
	TArray<FGimmickSongTiming> Songs = GimmickSongTiming::CollectSongTimings(Skipped);
	SkippedSongs = FString::Join(Skipped, TEXT("\n"));
	LevelSongs = GimmickSongTiming::FindSongsOfLevel(GetOpenLevelWorld());

	// The songs of the open level first, each group in the order of the song table
	Songs.StableSort([this](const FGimmickSongTiming& A, const FGimmickSongTiming& B) { return LevelSongs.Contains(A.SongTag) && !LevelSongs.Contains(B.SongTag); });

	SongOptions.Reset();
	for (FGimmickSongTiming& Song : Songs)
	{
		SongOptions.Add(MakeShared<FGimmickSongTiming>(MoveTemp(Song)));
	}
	SongOptions.Add(MakeShared<FGimmickSongTiming>());

	// The first song is the first song of the level. Without any song it is the entry for typing by hand
	TSharedPtr<FGimmickSongTiming> Initial = SongOptions[0];
	if (const FGameplayTag* Remembered = SongPerLevel.Find(GetOpenLevelKey()))
	{
		const TSharedPtr<FGimmickSongTiming>* Found = SongOptions.FindByPredicate([Remembered](const TSharedPtr<FGimmickSongTiming>& Song)
		{
			return Song->SongTag == *Remembered;
		});
		if (Found)
		{
			Initial = *Found;
		}
	}

	if (SongComboBox.IsValid())
	{
		SongComboBox->RefreshOptions();
		SongComboBox->SetSelectedItem(Initial);
	}
	SelectSong(Initial);
}

void SGimmickTimelineTab::SelectSong(const TSharedPtr<FGimmickSongTiming> Song)
{
	SelectedSong = Song;

	// Typing by hand starts from the values of the song picked before
	if (IsSong(Song))
	{
		TimelineRoundLength = Song->RoundLength;
		TimelineFeverStart = Song->FeverStart;
	}
	RebuildRows();
}

FText SGimmickTimelineTab::GetSongLabel(const TSharedPtr<FGimmickSongTiming>& Song) const
{
	if (!IsSong(Song))
	{
		return LOCTEXT("ManualSong", "직접 입력");
	}

	const FText Name = FText::FromString(Song->SongName);
	return LevelSongs.Contains(Song->SongTag) ? FText::Format(LOCTEXT("LevelSong", "{0}  (이 레벨)"), Name) : Name;
}

FString SGimmickTimelineTab::GetOpenLevelKey()
{
	const UWorld* World = GetOpenLevelWorld();
	return World ? World->GetPackage()->GetName() : FString();
}

EVisibility SGimmickTimelineTab::GetManualTimingVisibility() const
{
	return IsSong(SelectedSong) ? EVisibility::Collapsed : EVisibility::Visible;
}

#undef LOCTEXT_NAMESPACE
