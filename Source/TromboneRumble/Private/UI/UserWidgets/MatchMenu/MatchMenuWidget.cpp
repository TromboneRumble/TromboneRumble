// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/MatchMenu/MatchMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasyOnlineSession.h"
#include "EasyReservationManager.h"
#include "EasySessions.h"
#include "EasySessionStatics.h"
#include "TromboneGamePlayTags.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Framework/TromboneGameInstance.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "Online/OnlineSessionNames.h"
#include "UI/UserWidgets/Common/CommonButtonBaseExtensionWithText.h"
#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"
#include "Utilities/TromboneStatics.h"

void UMatchMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedMatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>();
	
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->OnMatchTypeChanged.RemoveAll(this);
		CachedMatchMenuGS->OnMatchTypeChanged.AddDynamic(this, &ThisClass::OnMatchTypeChanged);
		OnMatchTypeChanged(CachedMatchMenuGS->GetCurrentMatchType());

		CachedMatchMenuGS->OnSelectedMapChanged.RemoveAll(this);		
		CachedMatchMenuGS->OnSelectedMapChanged.AddDynamic(this, &ThisClass::OnSelectedMapChanged);
		OnSelectedMapChanged(CachedMatchMenuGS->GetSelectedLobbyMap());;
	}
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
		CR_MatchType->OnRotatedWithDirection().RemoveAll(this);
		CR_MatchType->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMatchType);
		CR_MatchType->SetIsEnabled(bIsHost);
	}
	if (CR_Map)
	{
		InitSelectableMaps();
		CR_Map->OnRotatedWithDirection().RemoveAll(this);
		CR_Map->OnRotatedWithDirection().AddDynamic(this, &ThisClass::HandleOnRotatedMap);
		CR_Map->SetIsEnabled(bIsHost);
		CR_Map->SetVisibility(ESlateVisibility::Hidden); // TODO : 매치메뉴에서 맵 선택 UI를 숨김. 추후 필요시 활성화
	}
}

void UMatchMenuWidget::InitSelectableMaps()
{
	CachedSelectableMaps.Reset();

	// 매치메뉴에서는 로비 맵들(Trombone.Maps.Lobby.*) 선택 가능
	const FGameplayTag LobbyCategory = FGameplayTag::RequestGameplayTag(FName(*TromboneGamePlayTags::LobbyPath), false);
	CachedSelectableMaps = UTromboneFunctionLibrary::GetMapTagsUnderCategory(LobbyCategory);

	if (!CR_Map)
	{
		return;
	}

	TArray<FText> Options;
	Options.Reserve(CachedSelectableMaps.Num());
	for (const FGameplayTag& MapTag : CachedSelectableMaps)
	{
		// "Trombone.Maps.Lobby.OrchestraStage" -> OrchestraStage를 UI에 표시되는 텍스트로 사용
		const FString TagStr = MapTag.ToString();
		FString Leaf;
		if (!TagStr.Split(TEXT("."), nullptr, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			Leaf = TagStr;
		}
		Options.Add(FText::FromString(Leaf));
	}
	CR_Map->SetOptions(Options);
}

void UMatchMenuWidget::OnMatchTypeChanged(EMatchType NewType)
{
	if (CR_MatchType)
	{
		const int32 Index = static_cast<int32>(NewType);
		CR_MatchType->SetSelectedIndex(Index);
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
	
	SetUIEnabled(false);
	
	bIsStarted = true;
	UEasyReservationManager* ReservationManager = UEasyReservationManager::Get(this);
	if (ReservationManager->IsReservationHost())
	{
		const TArray<FEasyReservation> Reservations = ReservationManager->CopyRegisteredReservations();
		ReservationManager->SetHostReservations(Reservations);
	}
	
	FEasySessionSettings UpdatedSettings;
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
			
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.bAllowJoinInProgress = false;

	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true);
	OnlineSession->StartOnlineSession(NAME_GameSession);
	
	FGameplayTag TargetMapTag = TromboneGamePlayTags::Trombone_Maps_Lobby_OrchestraStage;
	if (CachedMatchMenuGS.IsValid() && CachedMatchMenuGS->GetSelectedLobbyMap().IsValid())
	{
		TargetMapTag = CachedMatchMenuGS->GetSelectedLobbyMap();
	}

	const FString LobbyMapPath = UTromboneFunctionLibrary::GetMapPathByMapTag(TargetMapTag);
	UEasyStatics::ServerTravelToLevel(this, LobbyMapPath);
}

void UMatchMenuWidget::HandleBackButtonClicked()
{
	if (bIsStarted)
	{
		return;
	}
	
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	OnlineSession->DestroySession(NAME_GameSession);
	
	UTromboneStatics::OpenLevel(this, ELevelType::MainMenu);
}

void UMatchMenuWidget::HandleOnRotatedMatchType(int32 Value, ERotatorDirection RotatorDir)
{
	if (bIsStarted)
	{
		return;
	}
	
	UTromboneStatics::ShowLoadingOverlay(GetOwningPlayer());
	
	const EMatchType NewMatchType = static_cast<EMatchType>(Value);
	const bool bNewHidden = NewMatchType != EMatchType::Public;
	
	if (CachedMatchMenuGS.IsValid())
	{
		CachedMatchMenuGS->SetMatchType(NewMatchType);
	}
	
	FEasySessionSettings UpdatedSettings;
	UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this);
	
	OnlineSession->OnUpdateMatchComplete().AddDynamic(this, &ThisClass::HandleOnUpdateMatchComplete);
	OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);
	UpdatedSettings.bHidden = bNewHidden;
	
	TArray<FEasySessionSetting> ExtraSessionSettings = TArray<FEasySessionSetting>();
	
	FString LobbyCode;
	if (UpdatedSettings.GetSessionSetting(SETTING_LOBBYCODE, LobbyCode))
	{
		ExtraSessionSettings.Add(FEasySessionSetting(SETTING_LOBBYCODE, LobbyCode, EOnlineDataAdvertisementType::ViaOnlineService));
	}
			
	OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true, ExtraSessionSettings);
}

void UMatchMenuWidget::HandleOnRotatedMap(int32 Value, ERotatorDirection RotatorDir)
{
	if (bIsStarted || !CachedSelectableMaps.IsValidIndex(Value))
	{
		return;
	}

	const FGameplayTag SelectedLobbyTag = CachedSelectableMaps[Value];

	if (AMatchMenuGameState* MatchMenuGS = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGS->SetSelectedLobbyMap(SelectedLobbyTag);
	}

	const FGameplayTag InGameTag = UTromboneFunctionLibrary::LobbyToInGameTag(SelectedLobbyTag);
	if (InGameTag.IsValid())
	{
		if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
		{
			FEasySessionSettings UpdatedSettings;
			OnlineSession->GetSessionSettings(NAME_GameSession, UpdatedSettings);

			TArray<FEasySessionSetting> ExtraSessionSettings;
			ExtraSessionSettings.Add(FEasySessionSetting(SETTING_MAPNAME, InGameTag.ToString(), EOnlineDataAdvertisementType::ViaOnlineService));

			OnlineSession->UpdateSession(NAME_GameSession, UpdatedSettings, true, ExtraSessionSettings);
		}
	}
}

void UMatchMenuWidget::HandleOnUpdateMatchComplete(bool bWasSuccessful)
{
	UTromboneStatics::PopOverlay(GetOwningPlayer());
	
	if (bWasSuccessful)
	{
		if (UEasyOnlineSession* OnlineSession = UEasyOnlineSession::Get(this))
		{
			OnlineSession->OnUpdateMatchComplete().RemoveDynamic(this, &ThisClass::HandleOnUpdateMatchComplete);
		}
	}
}

void UMatchMenuWidget::SetUIEnabled(const bool bEnabled)
{
	CB_Start->SetIsEnabled(bEnabled);
	CB_Back->SetIsEnabled(bEnabled);
}