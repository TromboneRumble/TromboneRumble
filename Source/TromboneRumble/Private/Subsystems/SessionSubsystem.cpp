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
	// 자식에서 먼저 정리 시도 -> 부모 정리
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

	IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();

	// 기존 세션 있으면 먼저 삭제
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		//세션 종료시 재생성 플래그 설정
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastLobbyCode = LobbyCode;

		DestroySession();
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
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();

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
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const bool bLAN = IsLanEnvironment();

	if (bLAN)
	{
		int32 LocalUserNum = 0;
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
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();


	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : DestroySession failed from [DestroySession]"));
		OnSessionDestroyComplete.Broadcast(false);
	}
}

void USessionSubsystem::StartSession()
{
}
void USessionSubsystem::StartGameByPath(const FString& InMapPath)
{
	

}

bool USessionSubsystem::TryGetLobbyCode(FString& OutLobbyCode)
{

	//세션 설정에서 재조회
	if (IsValidSessionInterface())
	{
		IOnlineSessionPtr SI = SessionInterfaceWeak.Pin();
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



//void USessionSubsystem::CreateSession_Internal(const FString& InLobbyCode, int32 PublicConnections)
//{
//	IOnlineSessionPtr SI = GetSession();
//	if (!SI.IsValid())
//	{
//		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [CreateSession_Internal]"));
//		return;
//	}
//
//	const bool bLAN = IsLanEnvironment();
//
//	/// OSS 확인: LAN이면 NULL 허용, Online이면 Steam 요구
//	if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
//	{
//		const bool bIsNull = (OSS->GetSubsystemName() == "NULL");
//		if (!bLAN && bIsNull)
//		{
//			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Online host requires Steam, but OSS is NULL from [CreateSession_Internal]"));
//			return;
//		}
//		// bLAN && bIsNull 은 정상 (디버그용)
//	}
//	else
//	{
//		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No OnlineSubsystem for [CreateSession_Internal]"));
//		return;
//	}
//
//
//
//	// 기존 세션 있으면 먼저 삭제후 재시도
//	if (SI->GetNamedSession(SessionName))
//	{
//		auto DestroyHandleRef = MakeShared<FDelegateHandle>();
//		*DestroyHandleRef =
//			SI->AddOnDestroySessionCompleteDelegate_Handle(
//				FOnDestroySessionCompleteDelegate::CreateLambda(
//					[this, DestroyHandleRef, InLobbyCode, PublicConnections](FName, bool)
//					{
//						if (IOnlineSessionPtr S = GetSession(); S.IsValid())
//						{
//							S->ClearOnDestroySessionCompleteDelegate_Handle(*DestroyHandleRef);
//						}
//						CreateSession_Internal(InLobbyCode, PublicConnections);
//					})
//			);
//
//		if (!SI->DestroySession(SessionName))
//		{
//			// DestroySession 호출조차 실패하면 Delegate 바로 해제
//			SI->ClearOnDestroySessionCompleteDelegate_Handle(*DestroyHandleRef);
//			OnSessionError.Broadcast(TEXT("DestroySession failed to start."));
//		}
//		return;
//	}
//
//	CurrentLobbyCode = InLobbyCode;
//
//	FOnlineSessionSettings Settings;
//	Settings.bIsLANMatch = bLAN;
//	Settings.bIsDedicated = false;                 // 리슨 서버
//	Settings.bAllowJoinViaPresence = true;
//	Settings.bShouldAdvertise = true;
//	Settings.NumPublicConnections = FMath::Max(1, PublicConnections);
//	Settings.bUsesPresence = true;
//	Settings.bUseLobbiesIfAvailable = true;
//	Settings.bAllowJoinInProgress = true;
//	
//
//	// 키워드/로비코드 광고
//	FString MapName = GetWorld()->GetMapName();
//	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
//	Settings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
//	//Settings.Set(SEARCH_KEYWORDS, FString(TEXT("TRMB1")), EOnlineDataAdvertisementType::ViaOnlineService);
//
//	// 로비 코드 광고 (검색과 동일 키/타입)
//	Settings.Set(KEY_LOBBY_CODE, CurrentLobbyCode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
//
//	bool bStarted = false;
//
//	if (bLAN)
//	{
//		// LAN(NULL) 모드: NetId 없이 LocalUserNum 경로
//		const int32 LocalUserNum = 0;
//		bStarted = SI->CreateSession(LocalUserNum, SessionName, Settings);
//	}
//	else
//	{
//		// Online(Steam) 모드: NetId 필수
//		const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();
//		if (!LP || !LP->GetPreferredUniqueNetId().IsValid())
//		{
//			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No valid LocalPlayer/UniqueNetId for online host from [CreateSession_Internal]"));
//			return;
//		}
//		const FUniqueNetIdRepl NetId = LP->GetPreferredUniqueNetId();
//		bStarted = SI->CreateSession(*NetId, SessionName, Settings);
//	}
//
//	if (!bStarted)
//	{
//		OnSessionError.Broadcast(TEXT("CreateSession failed to start from [CreateSession_Internal]"));
//	}
//}


void USessionSubsystem::HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	OnSessionCreateComplete.Broadcast(bWasSuccessful);
}

void USessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	if (IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin())
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

	//// 로비 코드를 입력해서 찾은 경우
	//const FOnlineSessionSearchParam* Param = LastSessionSearch->QuerySettings.SearchParams.Find(KEY_LOBBY_CODE);
	//if (!Param)
	//{
	//	OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Missing KEY_LOBBY_CODE in QuerySettings from [HandleFindSessionsComplete]"));
	//	return;
	//}
	//const FString CodeParam = Param->Data.ToString();
	//if (CodeParam.IsEmpty())
	//{
	//	OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Empty KEY_LOBBY_CODE value from [HandleFindSessionsComplete]"));
	//	return;
	//}

	//// 찾은 Param에서 LobbyCode가 있는지 확인 
	//FOnlineSessionSearchResult Chosen;
	//if (!TryChooseResultByCode(CodeParam, Chosen))
	//{
	//	OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No result matched LobbyCode from [HandleFindSessionsComplete]"));
	//	return;
	//}
}

void USessionSubsystem::HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (IsValidSessionInterface())
	{
		IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	IOnlineSessionPtr SI = SessionInterfaceWeak.Pin();
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

		FString Reason = FString::Printf(TEXT("Join failed (%s). ResolvedURL ok? %d"),
			*ResultText, static_cast<int32>(bGot));
		OnSessionError.Broadcast(Reason);
	}



	OnSessionJoinComplete.Broadcast(Result);

}

void USessionSubsystem::HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (IsValidSessionInterface())
	{
		IOnlineSessionPtr SessionInterface = SessionInterfaceWeak.Pin();
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	// 세션 생성 시 기존 세션 있었을 경우 재생성
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastLobbyCode);
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
		if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
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


bool USessionSubsystem::TryChooseResultByCode(const FString& InLobbyCode, FOnlineSessionSearchResult& OutResult) const
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
