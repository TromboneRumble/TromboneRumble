// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickSettingsPanel.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "GimmickSettings/SGimmickIssueList.h"
#include "GimmickSettings/SGimmickPlayControls.h"
#include "GimmickSettings/SGimmickSharedSettingsTab.h"
#include "GimmickSettings/SGimmickTimelineTab.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "IDetailsView.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GimmickSettings"

using GimmickSettings::GetOpenLevelWorld;

void SGimmickSettingsPanel::Construct(const FArguments& InArgs)
{
	StageDetailsView = GimmickSettings::CreateDetailsView(false);

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f)
		[
			MakeTopBar()
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			MakeStatusArea()
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 8.f, 8.f)
		[
			MakeTabBar()
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SSeparator)
			.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
		]

		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			MakeTabContent()
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot().AutoHeight().MaxHeight(260.f).Padding(8.f)
		[
			SAssignNew(PlayControls, SGimmickPlayControls)
		]
	];

	MapOpenedHandle = FEditorDelegates::OnMapOpened.AddSP(this, &SGimmickSettingsPanel::HandleMapOpened);
	ObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SGimmickSettingsPanel::HandleObjectPropertyChanged);
	if (GEngine)
	{
		ActorAddedHandle = GEngine->OnLevelActorAdded().AddSP(this, &SGimmickSettingsPanel::HandleLevelActorChanged);
		ActorDeletedHandle = GEngine->OnLevelActorDeleted().AddSP(this, &SGimmickSettingsPanel::HandleLevelActorChanged);
	}

	UseStageDataOfOpenLevel();
}

SGimmickSettingsPanel::~SGimmickSettingsPanel()
{
	FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(ObjectPropertyChangedHandle);
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().Remove(ActorAddedHandle);
		GEngine->OnLevelActorDeleted().Remove(ActorDeletedHandle);
	}
}

void SGimmickSettingsPanel::PostUndo(const bool bSuccess)
{
	RequestRefresh();
}

void SGimmickSettingsPanel::PostRedo(const bool bSuccess)
{
	RequestRefresh();
}

void SGimmickSettingsPanel::RequestRefresh()
{
	if (bRefreshPending) return;

	bRefreshPending = true;
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SGimmickSettingsPanel::HandleRefreshTimer));
}

EActiveTimerReturnType SGimmickSettingsPanel::HandleRefreshTimer(double InCurrentTime, float InDeltaTime)
{
	bRefreshPending = false;
	Refresh();
	return EActiveTimerReturnType::Stop;
}

void SGimmickSettingsPanel::Refresh()
{
	UStageGimmickData* LevelData = FindStageDataOfOpenLevel();
	const bool bShowedLevelData = StageData.Get() == OpenLevelStageData.Get();
	OpenLevelStageData = LevelData;

	if (bShowedLevelData && StageData.Get() != LevelData)
	{
		SetStageData(LevelData);
		return;
	}

	RebuildSections();
	UpdateStatusText();
}

void SGimmickSettingsPanel::RebuildSections()
{
	const UWorld* World = GetOpenLevelWorld();

	// The level checks fit only the asset the open level uses
	const AGimmickManager* LevelManager = FindManagerOfOpenLevel();
	if (!StageData.IsValid() || !LevelManager || LevelManager->GetStageData() != StageData.Get())
	{
		LevelManager = nullptr;
	}

	IssueList->Rebuild(StageData.Get(), LevelManager);
	SharedSettingsTab->Rebuild(World);
	TimelineTab->Rebuild(StageData.Get());
	PlayControls->Rebuild(World, StageData.Get());
}

void SGimmickSettingsPanel::UpdateStatusText()
{
	const UWorld* World = GetOpenLevelWorld();
	const FString LevelName = World ? World->GetMapName() : TEXT("-");
	const UStageGimmickData* LevelData = OpenLevelStageData.Get();

	if (!LevelData)
	{
		StatusText = FText::Format(LOCTEXT("StatusNoData", "현재 레벨 {0}: 기믹 매니저가 없거나 기믹 매니저의 레벨 기믹 설정이 비어 있습니다."),
			FText::FromString(LevelName));
	}
	else if (LevelData != StageData.Get())
	{
		StatusText = FText::Format(LOCTEXT("StatusOtherData", "현재 레벨 {0}의 기믹 매니저는 {1}을 씁니다. 지금 보고 있는 설정은 이 레벨이 쓰는 것이 아니라서, 고쳐도 이 레벨의 플레이에는 반영되지 않습니다. 현재 레벨의 설정 버튼을 누르면 돌아갑니다."),
			FText::FromString(LevelName), FText::FromString(LevelData->GetName()));
	}
	else
	{
		StatusText = FText::Format(LOCTEXT("StatusSameData", "현재 레벨 {0}이 이 설정을 씁니다. 플레이 중에 고친 값은 대부분 그 기믹의 다음 발동이나 다음 스폰부터 쓰입니다. 바로 보려면 아래의 재시작을 누르세요. 저장을 눌러야 에셋에 남습니다."),
			FText::FromString(LevelName));
	}
}

void SGimmickSettingsPanel::HandleLevelActorChanged(AActor* Actor)
{
	// PIE spawns actors too, and the panel shows the editor level
	if (Actor && Actor->GetWorld() == GetOpenLevelWorld() && (Actor->IsA<AGimmickBase>() || Actor->IsA<AGimmickManager>()))
	{
		RequestRefresh();
	}
}

void SGimmickSettingsPanel::HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	// A dragged slider sends a change every frame, and the drag ends with one more change that is not interactive
	if (!Object || Event.ChangeType == EPropertyChangeType::Interactive) return;

	// The configs are subobjects of the asset, and the garbage class list of an entry decides what the shared data tab shows
	const bool bIsAsset = StageData.IsValid() && (Object == StageData.Get() || Object->IsIn(StageData.Get()));
	if (bIsAsset || Object->IsA<AGimmickManager>() || Object->IsA<AGimmickBase>())
	{
		RequestRefresh();
	}
}

void SGimmickSettingsPanel::HandleMapOpened(const FString& Filename, const bool bAsTemplate)
{
	TimelineTab->RefreshSongs();
	UseStageDataOfOpenLevel();
}

void SGimmickSettingsPanel::SetStageData(UStageGimmickData* NewStageData)
{
	StageData.Reset(NewStageData);
	StageDetailsView->SetObject(NewStageData);
	RebuildSections();
	UpdateStatusText();
}

void SGimmickSettingsPanel::UseStageDataOfOpenLevel()
{
	OpenLevelStageData = FindStageDataOfOpenLevel();
	SetStageData(OpenLevelStageData.Get());
}

void SGimmickSettingsPanel::HandleAssetPicked(const FAssetData& AssetData)
{
	SetStageData(Cast<UStageGimmickData>(AssetData.GetAsset()));
}

TArray<UPackage*> SGimmickSettingsPanel::GetEditedPackages() const
{
	TArray<UPackage*> Packages;
	if (StageData.IsValid())
	{
		Packages.AddUnique(StageData->GetPackage());
	}
	SharedSettingsTab->GetEditedPackages(Packages);
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

TSharedRef<SWidget> SGimmickSettingsPanel::MakeTopBar()
{
	return SNew(SHorizontalBox)

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
		];
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeStatusArea()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(this, &SGimmickSettingsPanel::GetStatusText)
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			SAssignNew(IssueList, SGimmickIssueList)
			.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
		];
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeTabBar()
{
	return SNew(SSegmentedControl<int32>)
		.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
		.Value_Lambda([this]() { return ActiveTabIndex; })
		.OnValueChanged_Lambda([this](const int32 NewIndex) { ActiveTabIndex = NewIndex; })

		+ SSegmentedControl<int32>::Slot(0)
		.Text(LOCTEXT("LevelTab", "레벨 설정"))
		.ToolTip(LOCTEXT("LevelTabTip", "이 레벨에서 쓰는 기믹과 기믹 시퀀스의 수치입니다. 스폰 규칙, 단계별 시간, 점수, 이 레벨에서 관중이 던질 투척물 종류 등. 위에서 고른 레벨 기믹 설정 에셋에 저장됩니다"))

		+ SSegmentedControl<int32>::Slot(1)
		.Text(LOCTEXT("SharedTab", "투척물 종류별 수치"))
		.ToolTip(LOCTEXT("SharedTabTip", "관중이 던지는 투척물 하나하나가 따로 갖는 수치입니다. 넉백, 피해 유형, 포물선 높이, 제거 지연. 현재 레벨의 관중 투척 항목에서 투척물 종류에 넣은 블루프린트만 보이고, 그 블루프린트에 저장됩니다. 위에서 다른 레벨의 설정을 골라도 이 탭은 현재 레벨의 투척물을 보여줍니다. 같은 투척물을 다른 레벨에서도 쓰면 그 레벨에도 적용됩니다"))

		+ SSegmentedControl<int32>::Slot(2)
		.Text(LOCTEXT("TimelineTab", "타임라인"))
		.ToolTip(LOCTEXT("TimelineTabTip", "레벨 설정의 수치대로라면 한 라운드 동안 기믹이 언제 발동하는지 보여줍니다. 보기만 하는 탭이고, 수치는 레벨 설정 탭에서 고칩니다"));
}

TSharedRef<SWidget> SGimmickSettingsPanel::MakeTabContent()
{
	return SNew(SWidgetSwitcher)
		.Visibility(this, &SGimmickSettingsPanel::GetStageDataAreaVisibility)
		.WidgetIndex_Lambda([this]() { return ActiveTabIndex; })

		+ SWidgetSwitcher::Slot()
		[
			StageDetailsView.ToSharedRef()
		]

		+ SWidgetSwitcher::Slot()
		[
			SAssignNew(SharedSettingsTab, SGimmickSharedSettingsTab)
		]

		+ SWidgetSwitcher::Slot()
		[
			SAssignNew(TimelineTab, SGimmickTimelineTab)
		];
}

EVisibility SGimmickSettingsPanel::GetStageDataAreaVisibility() const
{
	// Both tabs edit the selected asset or what it lists, so without one they would be two empty panes
	return StageData.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

AGimmickManager* SGimmickSettingsPanel::FindManagerOfOpenLevel()
{
	const UWorld* World = GetOpenLevelWorld();
	if (!World) return nullptr;

	for (TActorIterator<AGimmickManager> It(World); It; ++It)
	{
		if (It->GetStageData())
		{
			return *It;
		}
	}
	return nullptr;
}

UStageGimmickData* SGimmickSettingsPanel::FindStageDataOfOpenLevel()
{
	const AGimmickManager* Manager = FindManagerOfOpenLevel();

	// The game only reads the asset. This panel is the one place that writes it
	return Manager ? const_cast<UStageGimmickData*>(Manager->GetStageData()) : nullptr;
}

#undef LOCTEXT_NAMESPACE
