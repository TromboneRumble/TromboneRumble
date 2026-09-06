// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasyOnlineSession.h"
#include "EasyReservationManager.h"
#include "EasySessionStatics.h"
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
	const APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	const bool bIsHost = PC->HasAuthority();
	
	if (CB_Start)
	{
		CB_Start->OnClicked().RemoveAll(this);
		CB_Start->OnClicked().AddUObject(this, &ThisClass::HandleStartButtonClicked);
		CB_Start->SetIsEnabled(bIsHost);
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
		CR_MatchType->SetInteractionEnabled(bIsHost);
	}
	if (CR_Map)
	{
		InitSelectableMaps();
		CR_Map->OnRotatedWithDirection().RemoveAll(this);
		CR_Map->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMap);
		CR_Map->SetInteractionEnabled(bIsHost);
	}
	
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
	
	UEasyReservationManager* ReservationManager = UEasyReservationManager::Get(this);
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	if (!ReservationManager || !OnlineSession)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Session systems missing. Cannot start the match"));
		return;
	}
	
	SetUIEnabled(false);
	bIsStarted = true;
	
	// Confirm the current member list so the lobby starts with exactly these players
	if (ReservationManager->IsReservationHost())
	{
		const TArray<FEasyReservation> Reservations = ReservationManager->CopyRegisteredReservations();
		ReservationManager->SetHostReservations(Reservations);
	}
	
	// Start online session
	FEasySessionSettings UpdatedSettings;
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.bAllowJoinInProgress = false;
	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true);
	OnlineSession->StartOnlineSession(NAME_GameSession);
	
	// Travel to the picked lobby, or the default one if the game state is unreachable
	const FGameplayTag SelectedMap = CachedMatchMenuGS.IsValid() ? CachedMatchMenuGS->GetSelectedLobbyMap() : FGameplayTag();
	const FGameplayTag TargetMapTag = SelectedMap.IsValid() ? SelectedMap : TromboneGamePlayTags::Trombone_Maps_Lobby_OrchestraStage;
	UEasyStatics::ServerTravelToLevel(this, UTromboneFunctionLibrary::GetMapPathByMapTag(TargetMapTag));
}

void UMatchMenuWidget::HandleBackButtonClicked()
{
	if (bIsStarted)
	{
		return;
	}
	
	SetUIEnabled(false);
	
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	const bool bRequested = OnlineSession && OnlineSession->DestroySession(NAME_GameSession,
		FOnDestroySessionCompleteDelegate::CreateWeakLambda(this, [this](FName /*SessionName*/, bool /*bSuccess*/)
		{
			UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
		}));
	
	if (!bRequested)
	{
		UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
	}
}

void UMatchMenuWidget::HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir)
{
	if (bIsStarted || !CachedMatchTypes.IsValidIndex(Value))
	{
		return;
	}
	
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	if (!OnlineSession)
	{
		return;
	}
	
	const EMatchType NewMatchType = CachedMatchTypes[Value];
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->SetMatchType(NewMatchType);
	}
	
	// Custom rooms stay out of the public search list
	FEasySessionSettings UpdatedSettings;
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.bHidden = NewMatchType != EMatchType::Public;
	
	// Block input until the server answers
	UTromboneStatics::ShowLoadingOverlay(GetOwningPlayer());
	OnlineSession->OnUpdateMatchComplete().AddUniqueDynamic(this, &ThisClass::HandleOnUpdateMatchComplete);
	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true);
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
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	if (!InGameTag.IsValid() || !OnlineSession)
	{
		return;
	}

	FEasySessionSettings UpdatedSettings;
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.MapName = InGameTag.ToString();
	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true);
}

void UMatchMenuWidget::HandleOnUpdateMatchComplete(bool bWasSuccessful)
{
	UTromboneStatics::PopOverlay(GetOwningPlayer());
	
	if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
	{
		OnlineSession->OnUpdateMatchComplete().RemoveDynamic(this, &ThisClass::HandleOnUpdateMatchComplete);
	}
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