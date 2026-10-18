// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickPlayControls.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Styling/AppStyle.h"
#include "UObject/UObjectHash.h"
#include "Utilities/EnumHelper.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GimmickSettings"

namespace
{
	/** How a gimmick type runs, which decides what its Trigger, Stop and Restart buttons do. */
	enum class EGimmickKind : uint8 { Event, Spawner, Other };

	/** Kind of the gimmick type, read from the config class that carries it. A type without a config class, such as the blizzard, is Other. */
	EGimmickKind GetGimmickKind(const EGimmickType GimmickType)
	{
		TArray<UClass*> ConfigClasses;
		GetDerivedClasses(UGimmickConfig::StaticClass(), ConfigClasses);
		for (const UClass* Class : ConfigClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) continue;
			if (Class->GetDefaultObject<UGimmickConfig>()->GetGimmickType() != GimmickType) continue;

			if (Class->IsChildOf<UEventGimmickConfig>()) return EGimmickKind::Event;
			if (Class->IsChildOf<USpawnerGimmickConfig>()) return EGimmickKind::Spawner;
		}
		return EGimmickKind::Other;
	}

	/** Korean name of the gimmick type first, then the actor label so the designer can find it in the outliner. */
	FText MakeGimmickLabel(const AGimmickBase& Gimmick)
	{
		return FText::Format(INVTEXT("{0}  ({1})"), UEnum::GetDisplayValueAsText(Gimmick.GetGimmickType()), FText::FromString(Gimmick.GetActorLabel()));
	}

	/**
	 * Header bar of a bottom section. It uses the brush and the text style of a details category header,
	 * so it reads the same way as the category bars in the details view above it.
	 * The hint sits next to the title in a dimmer color.
	 */
	TSharedRef<SWidget> MakeSectionHeader(const FText& Title, const FText& Hint)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
			.Padding(FMargin(8.f, 4.f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Title)
					.TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
				]

				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Hint)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			];
	}
}

void SGimmickPlayControls::Construct(const FArguments& InArgs)
{
	using GimmickSettings::SectionRowIndent;

	ChildSlot
	[
		SNew(SScrollBox)

		+ SScrollBox::Slot()
		[
			// A level without gimmicks, such as the main menu, has no rows. A header over nothing is just noise
			SNew(SVerticalBox)
			.Visibility_Lambda([this]() { return GimmickRows.IsValid() && GimmickRows->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })

			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeSectionHeader(LOCTEXT("PieSection", "플레이 중 조작"), LOCTEXT("PieSectionHint", "PIE를 실행해야 버튼이 켜집니다"))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(SectionRowIndent, 4.f, 0.f, 12.f)
			[
				SAssignNew(GimmickRows, SVerticalBox)
			]
		]

		+ SScrollBox::Slot()
		[
			// Most levels have nothing here, and an empty section only raises the question of what is missing
			SNew(SVerticalBox)
			.Visibility_Lambda([this]() { return UnmanagedRows.IsValid() && UnmanagedRows->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })

			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeSectionHeader(LOCTEXT("UnmanagedSection", "이 창에서 고칠 수 없는 기믹"), LOCTEXT("UnmanagedSectionHint", "버튼으로 레벨의 액터를 선택해 Details에서 고칩니다"))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(SectionRowIndent, 4.f, 0.f, 0.f)
			[
				SAssignNew(UnmanagedRows, SVerticalBox)
			]
		]
	];
}

void SGimmickPlayControls::Rebuild(const UWorld* World, const UStageGimmickData* StageData)
{
	GimmickRows->ClearChildren();
	UnmanagedRows->ClearChildren();
	if (!World) return;

	TSet<EGimmickType> TypesWithRow;
	for (TActorIterator<AGimmickBase> It(World); It; ++It)
	{
		// The console commands find a gimmick by type, so every gimmick of the level gets buttons, with or without an entry
		GimmickRows->AddSlot().AutoHeight().Padding(0.f, 2.f)[MakeGimmickRow(MakeGimmickLabel(**It), It->GetGimmickType())];
		TypesWithRow.Add(It->GetGimmickType());

		// A gimmick with neither a level entry nor shared data has nothing the panel can show
		TArray<UObject*> SettingsObjects;
		It->GetSettingsObjects(SettingsObjects);
		const bool bHasEntry = StageData && StageData->FindConfig(It->GetGimmickType()) != nullptr;
		if (!bHasEntry && SettingsObjects.IsEmpty())
		{
			UnmanagedRows->AddSlot().AutoHeight().Padding(0.f, 2.f)[MakeUnmanagedRow(**It)];
		}
	}

	// A gimmick the manager spawns at runtime is not in the editor world, but it exists in PIE and takes the same commands
	if (StageData)
	{
		for (const UGimmickConfig* Config : StageData->GetGimmicks())
		{
			if (Config && Config->GetGimmickClass() && !TypesWithRow.Contains(Config->GetGimmickType()))
			{
				const FText Label = FText::Format(LOCTEXT("SpawnedRowLabel", "{0}  (매니저가 스폰)"), UEnum::GetDisplayValueAsText(Config->GetGimmickType()));
				GimmickRows->AddSlot().AutoHeight().Padding(0.f, 2.f)[MakeGimmickRow(Label, Config->GetGimmickType())];
				TypesWithRow.Add(Config->GetGimmickType());
			}
		}
	}
}

TSharedRef<SWidget> SGimmickPlayControls::MakeGimmickRow(const FText& Label, const EGimmickType GimmickType)
{
	FText TriggerTip;
	FText StopTip;
	FText RestartTip;
	switch (GetGimmickKind(GimmickType))
	{
	case EGimmickKind::Event:
		TriggerTip = LOCTEXT("TriggerEventTip", "기믹 시퀀스의 차례를 기다리지 않고 지금 발동합니다. 꺼져 있으면 켠 뒤에 발동합니다. 예고부터 시작하고, 예고가 없거나 예고 시간이 0이면 바로 발동합니다. 이미 진행 중이면 아무 일도 없고, 기믹 시퀀스의 차례는 바뀌지 않습니다");
		StopTip = LOCTEXT("StopEventTip", "기믹을 끕니다. 진행 중이면 바로 끝나고, 꺼져 있는 동안 기믹 시퀀스는 이 기믹의 차례를 건너뜁니다. 강제 발동이나 재시작을 누를 때까지 다시 켜지지 않습니다. 음악 큐가 모든 기믹을 다시 켜는 시점에는 함께 켜집니다");
		RestartTip = LOCTEXT("RestartEventTip", "껐다 켜서 고친 값을 바로 반영합니다. 진행 중이면 바로 끝나고, 기믹 시퀀스에서 이 기믹의 다음 차례에 고친 값으로 발동합니다");
		break;
	case EGimmickKind::Spawner:
		TriggerTip = LOCTEXT("TriggerSpawnerTip", "스폰 타이머를 기다리지 않고 지금 한 번 스폰합니다. 꺼져 있으면 켠 뒤에 스폰합니다. 개수는 지금 쓰는 스폰 규칙(일반 또는 피버)대로 뽑고, 스폰 타이머는 그대로 흘러갑니다");
		StopTip = LOCTEXT("StopSpawnerTip", "기믹을 끕니다. 예고 중인 스폰과 다음 스폰은 취소되고, 이미 스폰된 물체는 남습니다. 강제 발동이나 재시작을 누를 때까지 다시 켜지지 않습니다. 음악 큐가 모든 기믹을 다시 켜는 시점에는 함께 켜집니다");
		RestartTip = LOCTEXT("RestartSpawnerTip", "껐다 켜서 고친 값을 바로 반영합니다. 첫 스폰 지연부터 다시 셉니다");
		break;
	default:
		TriggerTip = LOCTEXT("TriggerOtherTip", "이 기믹은 강제 발동이 없어, 꺼져 있으면 켜기만 합니다");
		StopTip = LOCTEXT("StopOtherTip", "기믹을 끕니다. 강제 발동이나 재시작을 누를 때까지 다시 켜지지 않습니다. 음악 큐가 모든 기믹을 다시 켜는 시점에는 함께 켜집니다");
		RestartTip = LOCTEXT("RestartOtherTip", "껐다 켜서 고친 값을 바로 반영합니다");
		break;
	}

	const auto MakeCommandButton = [GimmickType](const FText& Label, const FText& ToolTip, const TCHAR* Command)
	{
		return SNew(SButton)
			.Text(Label)
			.ToolTipText(ToolTip)
			.IsEnabled_Static(&GimmickSettings::IsPlayingInEditor)
			.OnClicked_Lambda([Command, GimmickType]() { RunGimmickCommand(Command, GimmickType); return FReply::Handled(); });
	};

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(Label)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Trigger", "강제 발동"), TriggerTip, TEXT("Trombone.Gimmick.Trigger"))
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Stop", "강제 종료"), StopTip, TEXT("Trombone.Gimmick.Stop"))
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Restart", "재시작"), RestartTip, TEXT("Trombone.Gimmick.Restart"))
		];
}

TSharedRef<SWidget> SGimmickPlayControls::MakeUnmanagedRow(AGimmickBase& Gimmick)
{
	const TWeakObjectPtr<AGimmickBase> WeakGimmick = &Gimmick;
	const FText Label = MakeGimmickLabel(Gimmick);

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(Label)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectInLevel", "레벨에서 선택"))
			.OnClicked_Lambda([WeakGimmick]()
			{
				if (AGimmickBase* Target = WeakGimmick.Get(); Target && GEditor)
				{
					GEditor->SelectNone(false, true);
					GEditor->SelectActor(Target, true, true);
					GEditor->MoveViewportCamerasToActor(*Target, false);
				}
				return FReply::Handled();
			})
		];
}

void SGimmickPlayControls::RunGimmickCommand(const TCHAR* Command, const EGimmickType GimmickType)
{
	if (!GEngine) return;

	const FString CommandLine = FString::Printf(TEXT("%s %s"), Command, *EnumHelper::EnumToString(GimmickType));

	// Gimmicks run on the server. A client world would only log that it has no authority
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (Context.WorldType == EWorldType::PIE && World && World->GetNetMode() != NM_Client)
		{
			GEngine->Exec(World, *CommandLine);
		}
	}
}

#undef LOCTEXT_NAMESPACE
