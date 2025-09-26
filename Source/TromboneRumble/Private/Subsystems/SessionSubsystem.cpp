// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Package.h"
#include "Characters/DefaultCharacterController.h"
#include "ProtoType/PT_MainMenuPlayerController.h"

#include "Utilities/DebugHelper.h"

const FName USessionSubsystem::SessionName(TEXT("GameSession"));
const FName USessionSubsystem::KEY_LOBBY_CODE(TEXT("LOBBY_CODE"));

void USessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 엔진 전역에서 안전하게 세션 인터페이스 획득
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr Strong = OSS->GetSessionInterface();
		if (Strong.IsValid())
		{
			// 약참조로만 보관
			SessionInterfaceWeak = Strong;
		}
	}

	if (!SessionInterfaceWeak.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionInterface is invalid. Check OnlineSubsystem config."));
		return;
	}

	BindDelegates();
}

void USessionSubsystem::Deinitialize()
{
	// 자식에서 먼저 정리 시도 -> 부모 정리
	UnbindDelegates();
	SessionInterfaceWeak.Reset();
	Super::Deinitialize();
}

void USessionSubsystem::HostSessionWithRandomCode(int32 CodeLength, int32 PublicConnections)
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [HostSessionWithRandomCode]"));
		return;
	}
	CreateSession_Internal(GenerateRandomCode(FMath::Max(2, CodeLength)), PublicConnections);
}

void USessionSubsystem::HostSessionWithCode(const FString& InLobbyCode, int32 PublicConnections)
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [HostSessionWithCode]"));
		return;
	}
	if (InLobbyCode.IsEmpty())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Lobby code is empty from [HostSessionWithCode]"));
		return;
	}
	CreateSession_Internal(InLobbyCode, PublicConnections);
}

void USessionSubsystem::FindAndJoinByCode(const FString& InLobbyCode)
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [FindAndJoinByCode]"));
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 50; // 여유 있게 검색
	SessionSearch->bIsLanQuery = false;
	//TODO : Presence is deprecated. 대안 찾아야함.
	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(KEY_LOBBY_CODE, InLobbyCode, EOnlineComparisonOp::Equals);

	const int32 LocalUserNum = 0;
	if (!SI->FindSessions(LocalUserNum, SessionSearch.ToSharedRef()))
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : FindSessions failed to start from [FindAndJoinByCode]"));
	}
}

void USessionSubsystem::StartGameByPath(const FString& InMapPath)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : World is null from [StartGameByPath]"));
		return;
	}

	// 전달받은 문자열이 "오브젝트 경로(/A/B.Map.Map)"면 패키지 경로("/A/B.Map")로 정규화
	FString PackagePath = InMapPath;
	if (PackagePath.Contains(TEXT(".")))
	{
		PackagePath = FSoftObjectPath(InMapPath).GetLongPackageName();
	}

	//호스트면 직접 ServerTravel
	if (World->GetAuthGameMode() != nullptr) // 권한이 서버
	{
		const bool bOK = World->ServerTravel(PackagePath + TEXT("?listen"));
		if (!bOK)
		{ 
			OnSessionError.Broadcast(TEXT("SessionSubsystem Error : ServerTravel failed from [StartGameByPath]"));
		}
		return;
	}

	//클라면 PlayerController의 서버 RPC로 요청(얇은 포워더)
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		// 당신 프로젝트의 PC 클래스로 캐스팅
		if (auto* MPC = Cast<ADefaultCharacterController>(PC))
		{
			//MPC->Server_RequestStartGameByPath(PackagePath); // 아래 2) 참고
			//return;
		}
	}
	OnSessionError.Broadcast(TEXT("StartGame: No valid PlayerController"));
}

void USessionSubsystem::LeaveOrDestroySession()
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [LeaveOrDestroySession]"));
		return;
	}

	FNamedOnlineSession* Existing = SI->GetNamedSession(SessionName);
	if (!Existing)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No existing session from [LeaveOrDestroySession]"));
		return;
	}

	if (!SI->DestroySession(SessionName))
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : DestroySession failed to start from [LeaveOrDestroySession]"));
		Debug::Print(TEXT("DestroySession failed to start."));
	}
}

void USessionSubsystem::CreateSession_Internal(const FString& InLobbyCode, int32 PublicConnections)
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [CreateSession_Internal]"));
		return;
	}

	// LocalPlayer / NetId (스팀이면 NetId 사용, 아니면 LocalUserNum=0 사용)
	const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();
	const bool bHasValidNetId = (LP && LP->GetPreferredUniqueNetId().IsValid());
	if (!bHasValidNetId)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : No valid LocalPlayer/UniqueNetID from [CreateSession_Internal]"));
		return;
	}


	// 기존 세션 있으면 먼저 삭제후 재시도
	
	if (SI->GetNamedSession(SessionName))
	{
		auto DestroyHandleRef = MakeShared<FDelegateHandle>();
		*DestroyHandleRef =
			SI->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::CreateLambda(
					[this, DestroyHandleRef, InLobbyCode, PublicConnections](FName InSessionName_, bool bWasSuccessful)
					{
						// DestroyHandleRef 해제 후 CreateSession_Internal 재호출
						if (IOnlineSessionPtr S = GetSession(); S.IsValid())
						{
							S->ClearOnDestroySessionCompleteDelegate_Handle(*DestroyHandleRef);
						}

						CreateSession_Internal(InLobbyCode, PublicConnections);
					})
			);

		if (!SI->DestroySession(SessionName))
		{
			// DestroySession 호출조차 실패하면 Delegate 바로 해제
			SI->ClearOnDestroySessionCompleteDelegate_Handle(*DestroyHandleRef);
			OnSessionError.Broadcast(TEXT("DestroySession failed to start."));
		}
		return;
	}

	CurrentLobbyCode = InLobbyCode;

	FOnlineSessionSettings Settings;

	// 플랫폼에 따라 LAN 여부
	if (bForceLANForTesting)
	{
		Settings.bIsLANMatch = true;
	}
	else if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		Settings.bIsLANMatch = (OSS->GetSubsystemName() == "NULL");
	}
	else
	{
		Settings.bIsLANMatch = true; // 안전 기본값
	}

	Settings.bIsDedicated = false;                 // 리슨 서버
	Settings.bAllowJoinViaPresence = true;
	Settings.bShouldAdvertise = true;
	Settings.NumPublicConnections = FMath::Max(1, PublicConnections);
	Settings.bUsesPresence = true;
	Settings.bUseLobbiesIfAvailable = true;
	Settings.bAllowJoinInProgress = true;
	

	// (선택) 맵 이름 광고
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	Settings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);

	// 로비 코드 광고 (검색과 동일 키/타입)
	Settings.Set(KEY_LOBBY_CODE, CurrentLobbyCode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 세션 생성 완료시점에 호출될 람다 등록
	auto CreateHandleRef = MakeShared<FDelegateHandle>();
	*CreateHandleRef = SI->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateLambda(
			[this, CreateHandleRef](FName InSessionName, bool bWasSuccessful)
			{
				// 콜백 도착시점에 자기자신 해제
				if (IOnlineSessionPtr S = GetSession(); S.IsValid())
					S->ClearOnCreateSessionCompleteDelegate_Handle(*CreateHandleRef);

				if (!bWasSuccessful)
				{
					OnSessionError.Broadcast(TEXT("CreateSession failed."));
					return;
				}

				// UI가 이 이벤트를 받아 "방 코드 표시/대기"만 수행
				OnSessionCreated.Broadcast(CurrentLobbyCode);
			})
	);

	bool bStarted = false;
	if (LP->GetPreferredUniqueNetId().IsValid()) // 스팀/EOS 등
	{
		const FUniqueNetIdRepl NetId = LP->GetPreferredUniqueNetId();
		bStarted = SI->CreateSession(*NetId, SessionName, Settings);
	}
	else // 에디터/Standalone/LAN 테스트
	{
		const int32 LocalUserNum = 0;
		bStarted = SI->CreateSession(LocalUserNum, SessionName, Settings);
	}
	//세션 생성 시작 실패
	if (!bStarted) // 시작 실패 → 콜백 안 옴, 직접 해제
	{
		SI->ClearOnCreateSessionCompleteDelegate_Handle(*CreateHandleRef);
		OnSessionError.Broadcast(TEXT("CreateSession failed to start from [CreateSession_Internal]"));
	}
}

void USessionSubsystem::BindDelegates()
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error :IOnlineSessionPtr not valid from [BindDelegates]"));
		return;
	}

	CreateCompleteHandle = SI->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleCreateSessionComplete));

	FindCompleteHandle = SI->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFindSessionsComplete));

	JoinCompleteHandle = SI->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoinSessionComplete));

	DestroyCompleteHandle = SI->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroySessionComplete));
}

void USessionSubsystem::UnbindDelegates()
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [UnbindDelegates]"));
		return;
	}

	SI->ClearOnCreateSessionCompleteDelegate_Handle(CreateCompleteHandle);
	SI->ClearOnFindSessionsCompleteDelegate_Handle(FindCompleteHandle);
	SI->ClearOnJoinSessionCompleteDelegate_Handle(JoinCompleteHandle);
	SI->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteHandle);

	CreateCompleteHandle.Reset();
	FindCompleteHandle.Reset();
	JoinCompleteHandle.Reset();
	DestroyCompleteHandle.Reset();
}

void USessionSubsystem::HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : CreateSession failed from [HandleCreateSessionComplete]"));
		return;
	}

	OnSessionCreated.Broadcast(CurrentLobbyCode);
}

void USessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	bool bFoundAny = bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0;
	OnSessionSearchFinished.Broadcast(bFoundAny);
	if (!bFoundAny) return;

	const FOnlineSessionSearchParam* Param = SessionSearch->QuerySettings.SearchParams.Find(KEY_LOBBY_CODE);
	if (!Param)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Missing KEY_LOBBY_CODE in QuerySettings from [HandleFindSessionsComplete]"));
		return;
	}
	const FString CodeParam = Param->Data.ToString();

	FOnlineSessionSearchResult Chosen;
	if (!TryChooseResultByCode(CodeParam, Chosen))
		return;

	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid())
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : IOnlineSessionPtr not valid from [HandleFindSessionsComplete]"));
		return;
	}

	const int32 LocalUserNum = 0;
	if (!SI->JoinSession(LocalUserNum, SessionName, Chosen))
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : JoinSession failed to start from [HandleFindSessionsComplete]"));
	}
}

void USessionSubsystem::HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		OnSessionError.Broadcast(FString::Printf(TEXT("SessionSubsystem Error : JoinSession failed. Result=%d from [HandleJoinSessionComplete]"), (int32)Result));
		return;
	}

	FString URL;
	if (TryGetResolvedConnectString(URL))
	{
		OnSessionJoinURLReady.Broadcast(URL);
	}
	else
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : Failed to resolve connect string from [HandleJoinSessionComplete]"));
	}
}

void USessionSubsystem::HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnSessionError.Broadcast(TEXT("SessionSubsystem Error : DestroySession failed from [HandleDestroySessionComplete]"));
	}
	CurrentLobbyCode.Reset();
}

bool USessionSubsystem::TryChooseResultByCode(const FString& InLobbyCode, FOnlineSessionSearchResult& OutResult) const
{
	if (!SessionSearch.IsValid()) return false;

	for (const auto& R : SessionSearch->SearchResults)
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

bool USessionSubsystem::TryGetResolvedConnectString(FString& OutURL) const
{
	IOnlineSessionPtr SI = GetSession();
	if (!SI.IsValid()) return false;

	return SI->GetResolvedConnectString(SessionName, OutURL);
}

bool USessionSubsystem::IsLanEnvironment() const
{
	if (bForceLANForTesting)
		return true;


	if (const IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		return (OSS->GetSubsystemName() == "NULL"); // NULL 서브시스템이면 LAN
	}
	return true; // 안전 기본값: LAN
}

FString USessionSubsystem::GenerateRandomCode(int32 Length)
{
	static const TCHAR* Alphabet = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ23456789"); // O,0,I,1 제외
	const int32 N = FCString::Strlen(Alphabet);

	FString Result;
	Result.Reserve(Length);
	for (int32 i = 0; i < Length; ++i)
	{
		const int32 Index = FMath::RandRange(0, N - 1);
		Result.AppendChar(Alphabet[Index]);
	}
	return Result;
}
