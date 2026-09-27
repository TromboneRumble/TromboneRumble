// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickSettingsPanel.h"
#include "GimmickSettings/SGimmickTimeline.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Data/Gimmick/GimmickTimeline.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "IDetailRootObjectCustomization.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Utilities/EnumHelper.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GimmickSettings"

namespace
{
	const FName GimmickSettingMetaKey(TEXT("GimmickSetting"));

	UWorld* GetOpenLevelWorld()
	{
		return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}

	bool IsPlayingInEditor()
	{
		return GEditor && GEditor->PlayWorld != nullptr;
	}

	/** Korean name of the gimmick type first, then the actor label so the designer can find it in the outliner. */
	FText MakeGimmickLabel(const AGimmickBase& Gimmick)
	{
		return FText::Format(INVTEXT("{0}  ({1})"), UEnum::GetDisplayValueAsText(Gimmick.GetGimmickType()), FText::FromString(Gimmick.GetActorLabel()));
	}

	TSharedRef<SWidget> MakeSectionTitle(const FText& Title)
	{
		return SNew(STextBlock)
			.Text(Title)
			.Font(FAppStyle::GetFontStyle("BoldFont"));
	}

	/** Left indent of the rows under a section header, so a header and its rows do not look alike. */
	constexpr float SectionRowIndent = 16.f;

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

	/**
	 * A Blueprint holds meshes, sounds, components and feel values too. Only properties tagged with GimmickSetting are shown.
	 * The test must not depend on the object. Component properties of an actor arrive with the component as their object,
	 * and an earlier version that let every object except the class defaults through showed the transform and the Niagara settings.
	 */
	bool IsGimmickSettingProperty(const FPropertyAndParent& PropertyAndParent)
	{
		if (PropertyAndParent.Property.HasMetaData(GimmickSettingMetaKey)) return true;

		for (const FProperty* Parent : PropertyAndParent.ParentProperties)
		{
			if (Parent && Parent->HasMetaData(GimmickSettingMetaKey)) return true;
		}
		return false;
	}

	/** Header of one object in the shared data tab. Shows the Blueprint or asset name instead of Default__X_C. */
	class FSharedDataHeaderCustomization : public IDetailRootObjectCustomization
	{
	public:

		//~ Begin IDetailRootObjectCustomization Interface
		virtual TSharedPtr<SWidget> CustomizeObjectHeader(const FDetailsObjectSet& InRootObjectSet, const TSharedPtr<ITableRow>& InTableRow) override
		{
			const UObject* Object = InRootObjectSet.RootObjects.Num() > 0 ? InRootObjectSet.RootObjects[0] : nullptr;
			if (!Object) return SNullWidget::NullWidget;

			const bool bIsClassDefaults = Object->HasAnyFlags(RF_ClassDefaultObject);
			return MakeSectionTitle(bIsClassDefaults ? Object->GetClass()->GetDisplayNameText() : FText::FromString(Object->GetName()));
		}

		virtual bool AreObjectsVisible(const FDetailsObjectSet& InRootObjectSet) const override { return true; }
		virtual bool ShouldDisplayHeader(const FDetailsObjectSet& InRootObjectSet) const override { return true; }
		virtual EExpansionArrowUsage GetExpansionArrowUsage() const override { return EExpansionArrowUsage::Default; }
		//~ End IDetailRootObjectCustomization Interface
	};

	TSharedRef<IDetailsView> CreatePanelDetailsView(const bool bManyObjects)
	{
		FDetailsViewArgs DetailsArgs;
		DetailsArgs.bAllowMultipleTopLevelObjects = bManyObjects;
		DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsArgs.bAllowSearch = false;
		DetailsArgs.bHideSelectionTip = true;
		DetailsArgs.bUpdatesFromSelection = false;
		DetailsArgs.bLockable = false;
		DetailsArgs.bShowOptions = false;

		FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		return PropertyEditor.CreateDetailView(DetailsArgs);
	}
}

void SGimmickSettingsPanel::Construct(const FArguments& InArgs)
{
	StageDetailsView = CreatePanelDetailsView(false);

	SharedDetailsView = CreatePanelDetailsView(true);
	SharedDetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateStatic(&IsGimmickSettingProperty));
	// Rows added by detail customizations are not properties, so the filter above never sees them. Actors get several, such as the transform
	SharedDetailsView->SetIsCustomRowVisibleDelegate(FIsCustomRowVisible::CreateSP(this, &SGimmickSettingsPanel::IsSettingCategory));
	SharedDetailsView->SetRootObjectCustomizationInstance(MakeShared<FSharedDataHeaderCustomization>());

	// An entry added or removed in the details view changes the rows below
	StageDetailsView->OnFinishedChangingProperties().AddSP(this, &SGimmickSettingsPanel::HandlePropertiesChanged);

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("StageDataLabel", "레벨 기믹 설정"))
			]

			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UStageGimmickData::StaticClass())
				.ObjectPath_Lambda([this]() { return StageData.IsValid() ? StageData->GetPathName() : FString(); })
				.OnObjectChanged(this, &SGimmickSettingsPanel::HandleAssetPicked)
				.AllowClear(false)
				.DisplayThumbnail(false)
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("UseOpenLevel", "현재 레벨의 설정"))
				.ToolTipText(LOCTEXT("UseOpenLevelTip", "현재 레벨의 기믹 매니저가 가리키는 레벨 기믹 설정으로 바꿉니다"))
				.OnClicked_Lambda([this]() { UseStageDataOfOpenLevel(); return FReply::Handled(); })
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Save", "저장"))
				.ToolTipText(LOCTEXT("SaveTip", "두 탭에서 바뀐 것을 모두 저장합니다. 레벨 기믹 설정 에셋과, 고친 투척물 블루프린트가 저장됩니다"))
				.IsEnabled(this, &SGimmickSettingsPanel::HasUnsavedChanges)
				.OnClicked(this, &SGimmickSettingsPanel::HandleSaveClicked)
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(STextBlock)
			.Text(this, &SGimmickSettingsPanel::GetStatusText)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(SSegmentedControl<int32>)
			.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
			.Value_Lambda([this]() { return ActiveTabIndex; })
			.OnValueChanged_Lambda([this](const int32 NewIndex) { ActiveTabIndex = NewIndex; })

			+ SSegmentedControl<int32>::Slot(0)
			.Text(LOCTEXT("LevelTab", "레벨 설정"))
			.ToolTip(LOCTEXT("LevelTabTip", "기믹마다 하나씩 있는 수치입니다. 발동 간격, 지속 시간, 점수, 이 레벨에서 관중이 던질 투척물 종류 등. 위에서 고른 레벨 기믹 설정 에셋에 저장됩니다"))

			+ SSegmentedControl<int32>::Slot(1)
			.Text(LOCTEXT("SharedTab", "투척물 종류별 수치"))
			.ToolTip(LOCTEXT("SharedTabTip", "관중이 던지는 투척물 하나하나가 따로 갖는 수치입니다. 넉백, 피해 유형, 포물선 높이, 제거 지연. 현재 레벨의 관중 투척 항목에서 투척물 종류에 넣은 블루프린트만 보이고, 그 블루프린트에 저장됩니다. 위에서 다른 레벨의 설정을 골라도 이 탭은 현재 레벨의 투척물을 보여줍니다. 같은 투척물을 다른 레벨에서도 쓰면 그 레벨에도 적용됩니다"))

			+ SSegmentedControl<int32>::Slot(2)
			.Text(LOCTEXT("TimelineTab", "타임라인"))
			.ToolTip(LOCTEXT("TimelineTabTip", "레벨 설정의 수치대로라면 한 라운드 동안 기믹이 언제 발동하는지 보여줍니다. 보기만 하는 탭이고, 수치는 레벨 설정 탭에서 고칩니다"))
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SSeparator)
			.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
		]

		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SWidgetSwitcher)
			.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
			.WidgetIndex_Lambda([this]() { return ActiveTabIndex; })

			+ SWidgetSwitcher::Slot()
			[
				StageDetailsView.ToSharedRef()
			]

			+ SWidgetSwitcher::Slot()
			[
				SharedDetailsView.ToSharedRef()
			]

			+ SWidgetSwitcher::Slot()
			[
				MakeTimelineTab()
			]
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot().AutoHeight().MaxHeight(260.f).Padding(8.f)
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
		]
	];

	MapOpenedHandle = FEditorDelegates::OnMapOpened.AddSP(this, &SGimmickSettingsPanel::HandleMapOpened);

	UseStageDataOfOpenLevel();
}

SGimmickSettingsPanel::~SGimmickSettingsPanel()
{
	FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
}

EVisibility SGimmickSettingsPanel::GetStageDataAreaVisibility() const
{
	// Both tabs edit the selected asset or what it lists, so without one they would be two empty panes
	return StageData.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SGimmickSettingsPanel::SetStageData(UStageGimmickData* NewStageData)
{
	StageData.Reset(NewStageData);
	StageDetailsView->SetObject(NewStageData);
	RebuildLists();
	RebuildTimeline();
}

void SGimmickSettingsPanel::UseStageDataOfOpenLevel()
{
	SetStageData(FindStageDataOfOpenLevel());
}

UStageGimmickData* SGimmickSettingsPanel::FindStageDataOfOpenLevel()
{
	const UWorld* World = GetOpenLevelWorld();
	if (!World) return nullptr;

	for (TActorIterator<AGimmickManager> It(World); It; ++It)
	{
		if (const UStageGimmickData* Found = It->GetStageData())
		{
			// The game only reads the asset. This panel is the one place that writes it
			return const_cast<UStageGimmickData*>(Found);
		}
	}
	return nullptr;
}

void SGimmickSettingsPanel::RebuildLists()
{
	GimmickRows->ClearChildren();
	UnmanagedRows->ClearChildren();
	SharedObjects.Reset();

	const UWorld* World = GetOpenLevelWorld();
	if (!World)
	{
		SharedDetailsView->SetObjects(TArray<UObject*>());
		return;
	}

	TArray<UObject*> ObjectsToShow;
	TSet<EGimmickType> TypesWithRow;
	for (TActorIterator<AGimmickBase> It(World); It; ++It)
	{
		// The console commands find a gimmick by type, so every gimmick of the level gets buttons, with or without an entry
		const FText Label = MakeGimmickLabel(**It);
		GimmickRows->AddSlot().AutoHeight().Padding(0.f, 2.f)[MakeGimmickRow(Label, It->GetGimmickType())];
		TypesWithRow.Add(It->GetGimmickType());

		TArray<UObject*> SettingsObjects;
		It->GetSettingsObjects(SettingsObjects);

		for (UObject* Object : SettingsObjects)
		{
			if (Object)
			{
				ObjectsToShow.AddUnique(Object);
			}
		}

		// A gimmick with neither a level entry nor shared data has nothing this panel can show
		const bool bHasEntry = StageData.IsValid() && StageData->FindConfig(It->GetGimmickType()) != nullptr;
		if (!bHasEntry && SettingsObjects.IsEmpty())
		{
			UnmanagedRows->AddSlot().AutoHeight().Padding(0.f, 2.f)[MakeUnmanagedRow(**It)];
		}
	}

	// A gimmick the manager spawns at runtime is not in the editor world, but it exists in PIE and takes the same commands
	if (StageData.IsValid())
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

	// The layout is built inside SetObjects and asks IsSettingCategory while it does, so the categories must be known first
	SettingCategories.Reset();
	for (UObject* Object : ObjectsToShow)
	{
		SharedObjects.Add(Object);
		CollectSettingCategories(*Object->GetClass());
	}
	SharedDetailsView->SetObjects(ObjectsToShow);
}

void SGimmickSettingsPanel::RebuildTimeline()
{
	if (!Timeline.IsValid()) return;

	TArray<FGimmickTimelineRow> Rows;
	if (StageData.IsValid())
	{
		// One builder per config, in the order of the list. Sequences fill theirs below
		TArray<const UGimmickConfig*> Configs;
		TArray<FGimmickTimelineBuilder> Builders;
		for (const UGimmickConfig* Config : StageData->GetGimmicks())
		{
			if (!Config) continue;

			// Each row gets its own stream, so adding or editing one gimmick does not move the random waits of the others
			const int32 RowSeed = HashCombine(GetTypeHash(TimelineSeed), GetTypeHash(Config->GetGimmickType()));
			Configs.Add(Config);
			Builders.Emplace(TimelineRoundLength, TimelineFeverStart, RowSeed);

			if (StageData->FindSequence(Config->GetGimmickType())) continue;

			if (Config->RunsOnlyInSequence())
			{
				Builders.Last().SetNote(FText::FromString(TEXT("순서 그룹에 없어 발동하지 않습니다. 레벨 설정 탭의 순서 그룹에 넣어 주세요")));
				continue;
			}
			Config->BuildTimeline(Builders.Last());
		}

		const auto FindIndex = [&Configs](const EGimmickType Type) { return Configs.IndexOfByPredicate([Type](const UGimmickConfig* Config) { return Config->GetGimmickType() == Type; }); };

		// Repeat the rule of AGimmickManager: each gimmick starts a gap after the one before it ends
		for (const FGimmickSequence& Sequence : StageData->GetSequences())
		{
			if (Sequence.Order.IsEmpty()) continue;

			float Time = Sequence.FirstDelay;
			for (int32 Turn = 0; Time < TimelineRoundLength; ++Turn)
			{
				const int32 Index = FindIndex(Sequence.Order[Turn % Sequence.Order.Num()]);
				const float EventEnd = Configs.IsValidIndex(Index) ? Configs[Index]->BuildEventTimeline(Builders[Index], Time) : Time;
				Time = FMath::Max(EventEnd + Sequence.Gap, Time + FGimmickTimelineBuilder::MinStep);
			}

			FString OrderText;
			for (const EGimmickType Type : Sequence.Order)
			{
				OrderText += (OrderText.IsEmpty() ? TEXT("") : TEXT(" → ")) + UEnum::GetDisplayValueAsText(Type).ToString();
			}
			for (const EGimmickType Type : Sequence.Order)
			{
				const int32 Index = FindIndex(Type);
				if (!Builders.IsValidIndex(Index)) continue;

				const FText SequenceNote = FText::FromString(FString::Printf(TEXT("순서 그룹: %s 순서로 이어집니다"), *OrderText));
				Builders[Index].SetNote(Builders[Index].GetNote().IsEmpty() ? SequenceNote : FText::Format(INVTEXT("{0}\n{1}"), SequenceNote, Builders[Index].GetNote()));
			}
		}

		for (int32 Index = 0; Index < Configs.Num(); ++Index)
		{
			FGimmickTimelineRow& Row = Rows.AddDefaulted_GetRef();
			Row.Label = UEnum::GetDisplayValueAsText(Configs[Index]->GetGimmickType());
			Row.Note = Builders[Index].GetNote();
			Row.Spans = Builders[Index].GetSpans();
			Row.bUsesFever = Builders[Index].UsesFever();
		}
	}

	Timeline->SetRows(MoveTemp(Rows), TimelineRoundLength, TimelineFeverStart);
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeTimelineTab()
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

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("RoundLength", "라운드 길이"))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(80.f)
				[
					SNew(SSpinBox<float>)
					.MinValue(10.f)
					.MaxValue(600.f)
					.Delta(1.f)
					.ToolTipText(LOCTEXT("RoundLengthTip", "타임라인이 보여줄 길이(초)입니다. 기믹이 켜지는 시점부터 곡이 끝날 때까지의 시간을 넣습니다. 이 창에서만 쓰고 저장되지 않습니다"))
					.Value_Lambda([this]() { return TimelineRoundLength; })
					.OnValueChanged_Lambda([this](const float NewValue) { TimelineRoundLength = NewValue; RebuildTimeline(); })
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("FeverStart", "피버 시작"))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(80.f)
				[
					SNew(SSpinBox<float>)
					.MinValue(0.f)
					.MaxValue(600.f)
					.Delta(1.f)
					.ToolTipText(LOCTEXT("FeverStartTip", "곡의 피버 큐(Event_Spotlight_Fever)가 오는 시점(초)입니다. 스포트라이트만 이 시점부터 피버 간격을 씁니다. 이 창에서만 쓰고 저장되지 않습니다"))
					.Value_Lambda([this]() { return TimelineFeverStart; })
					.OnValueChanged_Lambda([this](const float NewValue) { TimelineFeverStart = NewValue; RebuildTimeline(); })
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Reroll", "다시 뽑기"))
				.ToolTipText(LOCTEXT("RerollTip", "간격이 최소~최대인 기믹(관중 투척, 스포트라이트, 중력)은 실제 게임에서 매번 다르게 나옵니다. 타임라인은 그중 한 가지 경우를 보여주며, 이 버튼으로 다른 경우를 봅니다. 간격이 고정인 기믹은 바뀌지 않습니다"))
				.OnClicked_Lambda([this]() { ++TimelineSeed; RebuildTimeline(); return FReply::Handled(); })
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
			.Text(LOCTEXT("TimelineNote", "0:00은 라운드 시작이 아니라 곡의 Event_Spotlight_Start 큐에서 기믹이 켜지는 시점입니다. 블록에 마우스를 올리면 단계와 시간이 보이고, 기믹 이름에 올리면 그 줄을 읽을 때 주의할 점이 보입니다. 눈보라는 이 창에서 관리하지 않아 나오지 않습니다"))
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
		];
}

void SGimmickSettingsPanel::CollectSettingCategories(const UClass& Class)
{
	for (TFieldIterator<FProperty> It(&Class); It; ++It)
	{
		if (!It->HasMetaData(GimmickSettingMetaKey)) continue;

		// The engine names each nested category node by its path so far (ObjectPropertyNode.cpp), so "Garbage|Config|Throw"
		// makes three nodes: "Garbage", "Garbage|Config" and "Garbage|Config|Throw". Every one of them is asked about,
		// and hiding any level hides everything below it. An earlier version added the single pieces but not the
		// middle path "Garbage|Config", which left the tab empty
		TArray<FString> Pieces;
		It->GetMetaData(TEXT("Category")).ParseIntoArray(Pieces, TEXT("|"), true);

		FString PathSoFar;
		for (const FString& Piece : Pieces)
		{
			if (!PathSoFar.IsEmpty())
			{
				PathSoFar += TEXT("|");
			}
			PathSoFar += Piece;
			SettingCategories.Add(FName(*PathSoFar));
		}
	}
}

bool SGimmickSettingsPanel::IsSettingCategory(const FName RowName, const FName ParentCategory) const
{
	// The engine asks this for custom rows and also for every subcategory. Returning false for all of them hid
	// the subcategories the settings live in, and the tab came up empty. The row name cannot tell the two apart,
	// because subcategories and most custom rows both pass None. The parent category can
	return SettingCategories.Contains(ParentCategory);
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeGimmickRow(const FText& Label, const EGimmickType GimmickType)
{

	const auto MakeCommandButton = [GimmickType](const FText& Label, const FText& ToolTip, const TCHAR* Command)
	{
		return SNew(SButton)
			.Text(Label)
			.ToolTipText(ToolTip)
			.IsEnabled_Static(&IsPlayingInEditor)
			.OnClicked_Lambda([Command, GimmickType]() { RunGimmickCommand(Command, GimmickType); return FReply::Handled(); });
	};

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(Label)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Trigger", "강제 발동"), LOCTEXT("TriggerTip", "타이머를 기다리지 않고 지금 발동합니다. 꺼져 있으면 켠 뒤에 발동합니다. 중력과 술통 침수는 예고 단계부터 시작하고, 취객은 예고 없이 바로 등장합니다. 이 셋은 이미 진행 중이면 아무 일도 없습니다. 관중 투척, 선물, 스포트라이트, 누수 물자국, 빙판은 누를 때마다 한 번 더 스폰합니다. 눈보라는 강제 발동이 없어 켜지기만 합니다"), TEXT("Trombone.Gimmick.Trigger"))
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Stop", "강제 종료"), LOCTEXT("StopTip", "기믹을 끕니다. 진행 중인 중력, 침수, 취객은 바로 끝납니다. 이미 스폰된 투척물, 선물, 웅덩이는 남습니다. 강제 발동이나 재시작을 누를 때까지 다시 발동하지 않습니다. 음악 큐가 모든 기믹을 다시 켜는 시점에는 함께 켜집니다"), TEXT("Trombone.Gimmick.Stop"))
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
		[
			MakeCommandButton(LOCTEXT("Restart", "재시작"), LOCTEXT("RestartTip", "껐다 켜서 고친 값을 바로 반영합니다. 첫 발동까지의 대기 시간부터 다시 셉니다"), TEXT("Trombone.Gimmick.Restart"))
		];
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeUnmanagedRow(AGimmickBase& Gimmick)
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

FText SGimmickSettingsPanel::GetStatusText() const
{
	const UWorld* World = GetOpenLevelWorld();
	const FString LevelName = World ? World->GetMapName() : TEXT("-");
	const UStageGimmickData* LevelData = FindStageDataOfOpenLevel();

	if (!LevelData)
	{
		return FText::Format(LOCTEXT("StatusNoData", "현재 레벨 {0}: 기믹 매니저가 없거나 기믹 매니저의 레벨 기믹 설정이 비어 있습니다."),
			FText::FromString(LevelName));
	}

	if (LevelData != StageData.Get())
	{
		return FText::Format(LOCTEXT("StatusOtherData", "현재 레벨 {0}의 기믹 매니저는 {1}을 씁니다. 지금 보고 있는 설정은 이 레벨이 쓰는 것이 아니라서, 고쳐도 이 레벨의 플레이에는 반영되지 않습니다. 현재 레벨의 설정 버튼을 누르면 돌아갑니다."),
			FText::FromString(LevelName), FText::FromString(LevelData->GetName()));
	}

	return FText::Format(LOCTEXT("StatusSameData", "현재 레벨 {0}이 이 설정을 씁니다. 플레이 중에 고친 값은 대부분 그 기믹의 다음 발동부터 쓰입니다. 바로 보려면 아래의 재시작을 누르세요. 저장을 눌러야 에셋에 남습니다."),
		FText::FromString(LevelName));
}

TArray<UPackage*> SGimmickSettingsPanel::GetEditedPackages() const
{
	TArray<UPackage*> Packages;
	if (StageData.IsValid())
	{
		Packages.AddUnique(StageData->GetPackage());
	}
	for (const TWeakObjectPtr<UObject>& Object : SharedObjects)
	{
		if (Object.IsValid())
		{
			Packages.AddUnique(Object->GetPackage());
		}
	}
	return Packages;
}

bool SGimmickSettingsPanel::HasUnsavedChanges() const
{
	for (const UPackage* Package : GetEditedPackages())
	{
		if (Package && Package->IsDirty()) return true;
	}
	return false;
}

FReply SGimmickSettingsPanel::HandleSaveClicked()
{
	UEditorLoadingAndSavingUtils::SavePackages(GetEditedPackages(), true);
	return FReply::Handled();
}

void SGimmickSettingsPanel::HandleAssetPicked(const FAssetData& AssetData)
{
	SetStageData(Cast<UStageGimmickData>(AssetData.GetAsset()));
}

void SGimmickSettingsPanel::HandleMapOpened(const FString& Filename, const bool bAsTemplate)
{
	UseStageDataOfOpenLevel();
}

void SGimmickSettingsPanel::HandlePropertiesChanged(const FPropertyChangedEvent& Event)
{
	// The garbage class list lives in the level entry and decides what the shared data tab shows
	RebuildLists();
	RebuildTimeline();
}

void SGimmickSettingsPanel::RunGimmickCommand(const TCHAR* Command, const EGimmickType GimmickType)
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
