// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasySessionStatics.h"
#include "EasySessionSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtension.h"
#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"
#include "Utilities/TromboneStatics.h"
#include "Utilities/DebugHelper.h"
#include "Data/MatchMenuRows.h"

UWidget* UMatchMenuWidget::NativeGetDesiredFocusTarget() const
{
	return CB_Start;
}

void UMatchMenuWidget::NativeDestruct()
{
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->OnMatchTypeChanged.RemoveAll(this);
		CachedMatchMenuGS->OnSelectedMapChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMatchMenuWidget::Init()
{
	if (!GetOwningPlayer()) return;
	
	if (CB_Start)
	{
		CB_Start->OnClicked().RemoveAll(this);
		CB_Start->OnClicked().AddUObject(this, &ThisClass::HandleStartButtonClicked);
	}
	if (CB_Back)
	{
		CB_Back->OnClicked().RemoveAll(this);
		CB_Back->OnClicked().AddUObject(this, &ThisClass::HandleBackButtonClicked);
	}
	if (CR_MatchType)
	{
		InitMatchTypes();
		CR_MatchType->OnRotatedWithDirection().RemoveAll(this);
		CR_MatchType->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMatchType);
	}
	if (CR_Map)
	{
		InitSelectableMaps();
		CR_Map->OnRotatedWithDirection().RemoveAll(this);
		CR_Map->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMap);
	}

	bIsStarted = false;
	SetUIEnabled(true);
	
	// 게임 스테이트 구독은 로테이터 초기화 뒤여야 한다. 초기 동기화가 위에서 채운 캐시로 인덱스를 찾기 때문
	CachedMatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>();
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->OnMatchTypeChanged.RemoveAll(this);
		CachedMatchMenuGS->OnMatchTypeChanged.AddDynamic(this, &ThisClass::OnMatchTypeChanged);
		OnMatchTypeChanged(CachedMatchMenuGS->GetCurrentMatchType());

		CachedMatchMenuGS->OnSelectedMapChanged.RemoveAll(this);
		CachedMatchMenuGS->OnSelectedMapChanged.AddDynamic(this, &ThisClass::OnSelectedMapChanged);
		OnSelectedMapChanged(CachedMatchMenuGS->GetSelectedLobbyMap());
	}
}

void UMatchMenuWidget::InitSelectableMaps()
{
	CachedSelectableMaps.Reset();

	TArray<FText> Options;
	TArray<FSlateBrush> Backgrounds;
	for (const MatchMenuOptions::FMapEntry& Entry : MatchMenuOptions::BuildSelectableMaps(SelectableMapTable))
	{
		CachedSelectableMaps.Add(Entry.Tag);
		Options.Add(Entry.Label);
		Backgrounds.Add(Entry.Background);
	}

	if (CR_Map)
	{
		CR_Map->SetOptions(Options, Backgrounds);
	}
}

void UMatchMenuWidget::InitMatchTypes()
{
	CachedMatchTypes.Reset();

	TArray<FText> Options;
	TArray<FSlateBrush> Backgrounds;
	for (const MatchMenuOptions::FMatchTypeEntry& Entry : MatchMenuOptions::BuildMatchTypes(MatchTypeTable))
	{
		CachedMatchTypes.Add(Entry.Type);
		Options.Add(Entry.Label);
		Backgrounds.Add(Entry.Background);
	}

	if (CR_MatchType)
	{
		CR_MatchType->SetOptions(Options, Backgrounds);
	}
}

void UMatchMenuWidget::OnMatchTypeChanged(EMatchType NewType)
{
	if (CR_MatchType)
	{
		const int32 Index = CachedMatchTypes.IndexOfByKey(NewType);
		if (Index != INDEX_NONE)
		{
			CR_MatchType->SetSelectedIndex(Index);
		}
	}
}

void UMatchMenuWidget::OnSelectedMapChanged(FGameplayTag NewMapTag)
{
	if (CR_Map)
	{
		const int32 Index = CachedSelectableMaps.IndexOfByKey(NewMapTag);
		if (Index != INDEX_NONE)
		{
			CR_Map->SetSelectedIndex(Index);
		}
	}
}

void UMatchMenuWidget::HandleStartButtonClicked()
{
	if (bIsStarted)
	{
		return;
	}
	
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Session systems missing. Cannot start the match"));
		return;
	}
	
	SetUIEnabled(false);
	bIsStarted = true;
	
	// The players here keep their reservations through the map change, and a started match without join-in-progress refuses anyone new
	FEasySessionSettings UpdatedSettings = Sessions->GetSessionSettings();
	UpdatedSettings.bAllowJoinInProgress = false;
	Sessions->UpdateSession(UpdatedSettings);
	
	// Travel to the picked lobby, or the default one if the game state is unreachable
	const FGameplayTag SelectedMap = CachedMatchMenuGS.IsValid() ? CachedMatchMenuGS->GetSelectedLobbyMap() : FGameplayTag();
	const FGameplayTag TargetMapTag = SelectedMap.IsValid() ? SelectedMap : TromboneGamePlayTags::Trombone_Maps_Lobby_OrchestraStage;
	const FString TargetMapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TargetMapTag);
	
	// Requests run in order on later ticks, so the travel waits for the start to finish
	Sessions->StartSession(FEasySessionCompleteDelegate::CreateWeakLambda(Sessions, [Sessions, TargetMapPath](EEasySessionResult Result, const FString& ErrorMessage)
	{
		if (Result != EEasySessionResult::Success)
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Starting the match failed (%s). Traveling anyway"), *ErrorMessage));
		}
		Sessions->ServerTravel(TargetMapPath);
	}));
}

void UMatchMenuWidget::HandleBackButtonClicked()
{
	if (bIsStarted)
	{
		return;
	}
	
	SetUIEnabled(false);
	
	// Leaving destroys the session and loads the Game Default Map, the main menu. A leaving host sends every player back too
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
		return;
	}
	Sessions->LeaveSession();
}

void UMatchMenuWidget::HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir)
{
	if (bIsStarted || !CachedMatchTypes.IsValidIndex(Value))
	{
		return;
	}
	
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!Sessions)
	{
		return;
	}
	
	const EMatchType NewMatchType = CachedMatchTypes[Value];
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->SetMatchType(NewMatchType);
	}
	
	// Custom rooms stay out of the public search list
	FEasySessionSettings UpdatedSettings = Sessions->GetSessionSettings();
	UpdatedSettings.bHidden = NewMatchType != EMatchType::Public;
	
	Sessions->UpdateSession(UpdatedSettings);
}

void UMatchMenuWidget::HandleOnRotatedMap(int32 Value, ERotatorDirection RotatorDir)
{
	if (bIsStarted || !CachedSelectableMaps.IsValidIndex(Value))
	{
		return;
	}

	const FGameplayTag SelectedLobbyTag = CachedSelectableMaps[Value];
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->SetSelectedLobbyMap(SelectedLobbyTag);
	}

	// The session advertises the in-game map so searching players see what they would play
	const FGameplayTag InGameTag = UTromboneFunctionLibrary::LobbyToInGameTag(SelectedLobbyTag);
	UEasySessionSubsystem* Sessions = UEasySessionStatics::GetEasySessionSubsystem(this);
	if (!InGameTag.IsValid() || !Sessions)
	{
		return;
	}

	FEasySessionSettings UpdatedSettings = Sessions->GetSessionSettings();
	UpdatedSettings.CustomSettings.Add(UTromboneStatics::InGameMapSettingKey, InGameTag.ToString());
	Sessions->UpdateSession(UpdatedSettings);
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	const APlayerController* PC = GetOwningPlayer();
	const bool bIsHost = PC && PC->HasAuthority();
	
	if (CB_Start)
	{
		CB_Start->SetIsEnabled(bEnabled && bIsHost);
	}
	if (CB_Back)
	{
		CB_Back->SetIsEnabled(bEnabled);
	}
	if (CR_MatchType)
	{
		CR_MatchType->SetInteractionEnabled(bEnabled && bIsHost);
	}
	if (CR_Map)
	{
		CR_Map->SetInteractionEnabled(bEnabled && bIsHost);
	}
}