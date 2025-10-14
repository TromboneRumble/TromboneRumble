// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Utilities/DebugHelper.h"

const FName USessionSubsystem::KEY_LOBBY_CODE(TEXT("LOBBY_CODE"));

USessionSubsystem::USessionSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleStartSessionComplete))
{
}

void USessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
	}
}

void USessionSubsystem::Deinitialize()
{
	SessionInterfaceWeak.Reset();
	
	Super::Deinitialize();
}

void USessionSubsystem::CreateSession(int32 NumPublicConnections, const FString& LobbyCode)
{
	if (!IsValidSessionInterface())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [CreateSession]"));
		return;
	}
	if (LobbyCode.IsEmpty())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : LobbyCode is empty from [CreateSession]"));
		return;
	}

	const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();

	const auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		RecreateSessionRequest.Emplace(NumPublicConnections, LobbyCode);
		DestroySession();
		return;
	}
	const bool bLAN = IsLanEnvironment();

	// Store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bLAN;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = !bLAN; //LAN 모드에서는 false
	LastSessionSettings->bUseLobbiesIfAvailable = !bLAN; //LAN 모드에서는 false

	/*맵 코드 광고시 주석 해제
	 FString MapName = GetWorld()->GetMapName();
	 MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	 Settings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	 */
	LastSessionSettings->Set(KEY_LOBBY_CODE, LobbyCode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//LastSessionSettings->BuildUniqueId = 1;

	bool bStarted = false;

	if (bLAN)
	{
		// LAN(NULL) 모드: NetId 없이 LocalUserNum 경로
		const int32 LocalUserNum = 0;
		bStarted = SessionInterface->CreateSession(LocalUserNum, NAME_GameSession, *LastSessionSettings);
	}
	else
	{
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
		{
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No valid LocalPlayer/UniqueNetId for online host from [CreateSession_Internal]"));
			return;
		}
		const FUniqueNetIdRepl NetId = LocalPlayer->GetPreferredUniqueNetId();
		bStarted = SessionInterface->CreateSession(*NetId, NAME_GameSession, *LastSessionSettings);
	}
	if (!bStarted)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnSessionCreateComplete.Broadcast(false);
		OnSessionError.Broadcast(TEXT("CreateSession failed to start from [CreateSession_Internal]"));
	}
	
}

void USessionSubsystem::FindSessions(int32 MaxSearchResults, const FString& InLobbyCode)
{
	if (!IsValidSessionInterface())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [FindSessions]"));
		return;
	}
	const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	const bool bLan = IsLanEnvironment();

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bLan;	
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, !bLan, EOnlineComparisonOp::Equals);
	
	if (!InLobbyCode.IsEmpty())
	{
		LastSessionSearch->QuerySettings.Set(KEY_LOBBY_CODE, InLobbyCode, EOnlineComparisonOp::Equals);
	}

	if (bLan)
	{
		const int32 LocalUserNum = 0;
		if (!SessionInterface->FindSessions(LocalUserNum, LastSessionSearch.ToSharedRef()))
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : FindSessions failed from [FindSessions]"));
			OnSessionSearchFinished.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		}
	}
	else
	{
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : FindSessions failed from [FindSessions]"));
			OnSessionSearchFinished.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		}
	}

	
}

void USessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!IsValidSessionInterface())
	{
		OnSessionJoinComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [CreateSession]"));
		return;
	}
	const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const bool bLAN = IsLanEnvironment();

	if (bLAN)
	{
		const int32 LocalUserNum = 0;
		if (!SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, SessionResult))
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : JoinSession failed to start from [HandleFindSessionsComplete]"));
			OnSessionJoinComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		}
	}
	else
	{
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : JoinSession failed to start from [HandleFindSessionsComplete]"));
			OnSessionJoinComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		}
	}
}

void USessionSubsystem::DestroySession()
{
	if (!IsValidSessionInterface())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [DestroySession]"));
		OnSessionDestroyComplete.Broadcast(false);
		return;
	}
	const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();


	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : DestroySession failed from [DestroySession]"));
		OnSessionDestroyComplete.Broadcast(false);
	}
}

bool USessionSubsystem::TryGetCurrentLobbyCode(FString& OutLobbyCode)
{

	//세션 설정에서 재조회
	if (IsValidSessionInterface())
	{
		const IOnlineSessionPtr SI = SessionInterfaceWeak.Pin();
		if (const FNamedOnlineSession* Named = SI->GetNamedSession(NAME_GameSession))
		{
			FString Found;
			if (Named->SessionSettings.Get(KEY_LOBBY_CODE, Found) && !Found.IsEmpty())
			{
				OutLobbyCode = Found;
				return true;
			}
		}
	}

	OutLobbyCode.Reset();
	return false;
}

bool USessionSubsystem::IsLocalHost() const
{
	if (const UWorld* World = GetWorld())
	{
		const ENetMode NM = World->GetNetMode();
		// 리슨 서버(호스트) 또는 전용 서버일 때 true
		return (NM == NM_ListenServer || NM == NM_DedicatedServer);
	}
	return false;
}


void USessionSubsystem::HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	OnSessionCreateComplete.Broadcast(bWasSuccessful);
}

void USessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	if (const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	const bool bSearchValid = LastSessionSearch.IsValid();
	const int32 NumResults = bSearchValid ? LastSessionSearch->SearchResults.Num() : 0;
	const bool bFoundAny = bWasSuccessful && bSearchValid && NumResults > 0;

	if (!bWasSuccessful)
	{
		OnSessionSearchFinished.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : FindSessions finished with failure from [HandleFindSessionsComplete]"));
		return;
	}

	if (!bSearchValid)
	{
		OnSessionSearchFinished.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : SessionSearch invalid from [HandleFindSessionsComplete]"));
		return;
	}
	if (NumResults <= 0)
	{
		OnSessionSearchFinished.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		const FString Reason = FString::Printf(
			TEXT("SessionSubsystem Error : No sessions found (bIsLanQuery=%d) from [HandleFindSessionsComplete]"),
			(int32)LastSessionSearch->bIsLanQuery);
		OnSessionError.Broadcast(Reason);
		return;
	}

	OnSessionSearchFinished.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void USessionSubsystem::HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (IsValidSessionInterface())
	{
		const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	const IOnlineSessionPtr SI = SessionInterfaceWeak.Pin();
	FString ConnectString;
	const bool bGot = SI.IsValid() ? SI->GetResolvedConnectString(InSessionName, ConnectString) : false;
	Debug::Print(FString::Printf(TEXT("GetResolvedConnectString=%s, URL=%s"),
		bGot ? TEXT("true") : TEXT("false"),
		*ConnectString));
	if (Result == EOnJoinSessionCompleteResult::Success && bGot)
	{
	}
	else
	{
		// CouldNotRetrieveAddress로 매핑 안 되는 경우가 있어 수동 메시지
		FString ResultText;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::Success:                 ResultText = TEXT("Success"); break;
		case EOnJoinSessionCompleteResult::SessionIsFull:           ResultText = TEXT("SessionIsFull"); break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:     ResultText = TEXT("SessionDoesNotExist"); break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: ResultText = TEXT("CouldNotRetrieveAddress"); break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:        ResultText = TEXT("AlreadyInSession"); break;
		default:                                                    ResultText = TEXT("Unknown"); break;
		}

		const FString Reason = FString::Printf(TEXT("Join failed (%s). ResolvedURL ok? %d"),
			*ResultText, static_cast<int32>(bGot));
		OnSessionError.Broadcast(Reason);
	}



	OnSessionJoinComplete.Broadcast(Result);

}

void USessionSubsystem::HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (IsValidSessionInterface())
	{
		const IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	
	if (bWasSuccessful && RecreateSessionRequest.IsSet())
	{
		const FRecreateSessionRequest Request = RecreateSessionRequest.GetValue();
		CreateSession(Request.NumPublicConnections, Request.LobbyCode);
		RecreateSessionRequest.Reset();
	}
	OnSessionDestroyComplete.Broadcast(bWasSuccessful);
}

void USessionSubsystem::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void USessionSubsystem::HandleNetworkFailure(UWorld* InWorld, UNetDriver* InNetDriver,
                                             ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const FString Msg = FString::Printf(TEXT("NetworkFailure: %d %s"), (int32)FailureType, *ErrorString);
	OnSessionError.Broadcast(Msg);
}

void USessionSubsystem::HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType,
	const FString& ErrorString)
{
	const FString Msg = FString::Printf(TEXT("TravelFailure: %d %s"), (int32)FailureType, *ErrorString);
	OnSessionError.Broadcast(Msg);
}

bool USessionSubsystem::IsValidSessionInterface()
{

	IOnlineSessionPtr SessionInterface = nullptr;
	if (!SessionInterfaceWeak.IsValid())
	{
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			SessionInterface = Subsystem->GetSessionInterface();
			if (SessionInterface)
			{
				SessionInterfaceWeak = SessionInterface;
			}
			
			SessionInterface = Subsystem->GetSessionInterface();
		}
	}
	else
	{
		SessionInterface = SessionInterfaceWeak.Pin();
	}
	return SessionInterface.IsValid();
}


bool USessionSubsystem::FindMatchingLobbyInResult(const FString& InLobbyCode, FOnlineSessionSearchResult& OutResult) const
{
	if (!LastSessionSearch.IsValid()) return false;

	for (const auto& R : LastSessionSearch->SearchResults)
	{
		FString FoundCode;
		if (R.Session.SessionSettings.Get(KEY_LOBBY_CODE, FoundCode))
		{
			if (FoundCode.Equals(InLobbyCode, ESearchCase::IgnoreCase))
			{
				OutResult = R;
				return true;
			}
		}
	}
	return false;
}

bool USessionSubsystem::IsLanEnvironment() const
{
#if WITH_EDITOR
	if (FParse::Param(FCommandLine::Get(), TEXT("nosteam")) ||
		FParse::Param(FCommandLine::Get(), TEXT("tr_lan")))
	{
		return true;
	}
#endif
	// 기본은 Online(= Steam)
	return IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
}
