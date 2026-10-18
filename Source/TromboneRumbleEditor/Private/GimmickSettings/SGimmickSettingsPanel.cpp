// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "GimmickSettings/SGimmickSettingsPanel.h"
#include "GimmickSettings/GimmickSettingsCommon.h"
#include "GimmickSettings/SGimmickIssueList.h"
#include "GimmickSettings/SGimmickPlayControls.h"
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

using GimmickSettings::GetCurrentLevelWorld;

void SGimmickSettingsPanel::Construct(const FArguments& InArgs)
{
	StageDetailsView = GimmickSettings::CreateDetailsView();

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
	LevelLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddSP(this, &SGimmickSettingsPanel::HandleLevelLoaded);
	PlayStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(this, &SGimmickSettingsPanel::HandlePlayStartedOrEnded);
	PlayEndedHandle = FEditorDelegates::EndPIE.AddSP(this, &SGimmickSettingsPanel::HandlePlayStartedOrEnded);
	ObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SGimmickSettingsPanel::HandleObjectPropertyChanged);
	if (GEngine)
	{
		ActorAddedHandle = GEngine->OnLevelActorAdded().AddSP(this, &SGimmickSettingsPanel::HandleLevelActorChanged);
		ActorDeletedHandle = GEngine->OnLevelActorDeleted().AddSP(this, &SGimmickSettingsPanel::HandleLevelActorChanged);
	}

	UseStageDataOfCurrentLevel();
}

SGimmickSettingsPanel::~SGimmickSettingsPanel()
{
	FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadedHandle);
	FEditorDelegates::PostPIEStarted.Remove(PlayStartedHandle);
	FEditorDelegates::EndPIE.Remove(PlayEndedHandle);
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

void SGimmickSettingsPanel::RequestRefresh(const bool bLevelChanged)
{
	bLevelChangePending |= bLevelChanged;
	if (bRefreshPending) return;

	bRefreshPending = true;
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SGimmickSettingsPanel::HandleRefreshTimer));
}

EActiveTimerReturnType SGimmickSettingsPanel::HandleRefreshTimer(double InCurrentTime, float InDeltaTime)
{
	bRefreshPending = false;

	// The songs of the timeline depend on the level
	if (bLevelChangePending)
	{
		bLevelChangePending = false;
		TimelineTab->RefreshSongs();
	}

	Refresh();
	return EActiveTimerReturnType::Stop;
}

void SGimmickSettingsPanel::Refresh()
{
	UStageGimmickData* LevelData = FindStageDataOfCurrentLevel();
	const bool bShowedLevelData = StageData.Get() == CurrentLevelStageData.Get();
	CurrentLevelStageData = LevelData;

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
	const UWorld* World = GetCurrentLevelWorld();

	// The level checks fit only the asset the current level uses
	const AGimmickManager* LevelManager = FindManagerOfCurrentLevel();
	if (!StageData.IsValid() || !LevelManager || LevelManager->GetStageData() != StageData.Get())
	{
		LevelManager = nullptr;
	}

	IssueList->Rebuild(StageData.Get(), LevelManager);
	TimelineTab->Rebuild(StageData.Get());
	PlayControls->Rebuild(World, StageData.Get());
}

void SGimmickSettingsPanel::UpdateStatusText()
{
	const UWorld* World = GetCurrentLevelWorld();
	const FString LevelName = World ? World->GetMapName() : TEXT("-");
	const UStageGimmickData* LevelData = CurrentLevelStageData.Get();

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
	// Only the current level matters. The editor world during PIE and the client worlds of PIE have their own actors
	if (Actor && Actor->GetWorld() == GetCurrentLevelWorld() && (Actor->IsA<AGimmickBase>() || Actor->IsA<AGimmickManager>()))
	{
		RequestRefresh();
	}
}

void SGimmickSettingsPanel::HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	// A dragged slider sends a change every frame, and the drag ends with one more change that is not interactive
	if (!Object || Event.ChangeType == EPropertyChangeType::Interactive) return;

	// The configs are subobjects of the asset
	const bool bIsAsset = StageData.IsValid() && (Object == StageData.Get() || Object->IsIn(StageData.Get()));
	if (bIsAsset || Object->IsA<AGimmickManager>() || Object->IsA<AGimmickBase>())
	{
		RequestRefresh();
	}
}

void SGimmickSettingsPanel::HandleMapOpened(const FString& Filename, const bool bAsTemplate)
{
	RequestRefresh(true);
}

void SGimmickSettingsPanel::HandleLevelLoaded(UWorld* LoadedWorld)
{
	// The editor opens its maps through HandleMapOpened. A seamless travel in PIE ends here too
	if (LoadedWorld && LoadedWorld->WorldType == EWorldType::PIE)
	{
		RequestRefresh(true);
	}
}

void SGimmickSettingsPanel::HandlePlayStartedOrEnded(const bool bIsSimulating)
{
	// When PIE ends its world is still there. The refresh runs on the next frame, after PIE has let go of it
	RequestRefresh(true);
}

void SGimmickSettingsPanel::SetStageData(UStageGimmickData* NewStageData)
{
	StageData.Reset(NewStageData);
	StageDetailsView->SetObject(NewStageData);
	RebuildSections();
	UpdateStatusText();
}

void SGimmickSettingsPanel::UseStageDataOfCurrentLevel()
{
	CurrentLevelStageData = FindStageDataOfCurrentLevel();
	SetStageData(CurrentLevelStageData.Get());
}

void SGimmickSettingsPanel::HandleAssetPicked(const FAssetData& AssetData)
{
	SetStageData(Cast<UStageGimmickData>(AssetData.GetAsset()));
}

bool SGimmickSettingsPanel::HasUnsavedChanges() const
{
	return StageData.IsValid() && StageData->GetPackage()->IsDirty();
}

FReply SGimmickSettingsPanel::HandleSaveClicked()
{
	if (StageData.IsValid())
	{
		UEditorLoadingAndSavingUtils::SavePackages({ StageData->GetPackage() }, true);
	}
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
			.OnClicked_Lambda([this]() { UseStageDataOfCurrentLevel(); return FReply::Handled(); })
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Save", "저장"))
			.ToolTipText(LOCTEXT("SaveTip", "레벨 기믹 설정 에셋에서 바뀐 것을 저장합니다"))
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
			SAssignNew(TimelineTab, SGimmickTimelineTab)
		];
}

EVisibility SGimmickSettingsPanel::GetStageDataAreaVisibility() const
{
	// Both tabs show the selected asset, so without one they would be two empty panes
	return StageData.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

AGimmickManager* SGimmickSettingsPanel::FindManagerOfCurrentLevel()
{
	const UWorld* World = GetCurrentLevelWorld();
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

UStageGimmickData* SGimmickSettingsPanel::FindStageDataOfCurrentLevel()
{
	const AGimmickManager* Manager = FindManagerOfCurrentLevel();

	// The game only reads the asset. This panel is the one place that writes it
	return Manager ? const_cast<UStageGimmickData*>(Manager->GetStageData()) : nullptr;
}

#undef LOCTEXT_NAMESPACE
