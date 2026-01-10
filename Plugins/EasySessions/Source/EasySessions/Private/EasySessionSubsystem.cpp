// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySessionSubsystem.h"
#include "EasySessionUtils.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"

UEasySessionSubsystem::UEasySessionSubsystem() :
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
}

void UEasySessionSubsystem::CreateSession(const FEasySessionSettings& InSettings)
{
    FEasyOnlineHelper Helper(TEXT("CreateSession"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            LastSettings = InSettings;

            if (Sessions->GetNamedSession(NAME_GameSession))
            {
                DestroySession();
                return;
            }

            CreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
            
            FOnlineSessionSettings Settings;
            Settings.NumPublicConnections = InSettings.NumPublicConnections;
            Settings.NumPrivateConnections = InSettings.NumPrivateConnections;
            Settings.bShouldAdvertise = InSettings.bShouldAdvertise;
            Settings.bAllowJoinInProgress = InSettings.bAllowJoinInProgress;
            Settings.bIsLANMatch = InSettings.bIsLAN;
            Settings.bAllowJoinViaPresence = InSettings.bAllowJoinViaPresence;
            Settings.bIsDedicated = InSettings.bIsDedicated;
            
            Settings.bUseLobbiesVoiceChatIfAvailable = InSettings.bUseLobbiesIfAvailable ? InSettings.bUseLobbiesVoiceChatIfAvailable : false;
            Settings.bAllowJoinViaPresenceFriendsOnly = InSettings.bAllowJoinViaPresenceFriendsOnly;
            Settings.bAntiCheatProtected = InSettings.bAntiCheatProtected;
            Settings.bUsesStats = InSettings.bUsesStats;
            Settings.bAllowInvites = InSettings.bAllowInvites;
            
            if (InSettings.bIsDedicated)
            {
                Settings.bUseLobbiesIfAvailable = false;
                Settings.bUsesPresence = false;
            }
            else
            {
                Settings.bUseLobbiesIfAvailable = InSettings.bUseLobbiesIfAvailable;
                Settings.bUsesPresence = InSettings.bUseLobbiesIfAvailable;
            }

            for (const auto& Pair : InSettings.CustomProperties)
            {
                FOnlineSessionSetting ExtraSetting;
                ExtraSetting.Data = Pair.Value;
                ExtraSetting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
                
                Settings.Set(FName(*Pair.Key), ExtraSetting);
            }
            
            if (!InSettings.bIsDedicated)
            {
                Sessions->CreateSession(*Helper.UserID, NAME_GameSession, Settings);
            }
            else
            {
                Sessions->CreateSession(0, NAME_GameSession, Settings);
            }
            
            return;
        }
        else
        {
            UE_LOG_ONLINE_SESSION(Display, TEXT("[EasySession] Cannot host session: Session Interface is invalid"));
        }
    }
    
    OnStartSessionFailure.Broadcast();
}

void UEasySessionSubsystem::OnCreateSessionComplete(FName SessionName, const bool bWasSuccessful)
{
    const FEasyOnlineHelper Helper(TEXT("CreateSessionCallback"), GetWorld());
    
    if (Helper.OnlineSub != nullptr)
    {
        const auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
            
            if (bWasSuccessful)
            {
                if (LastSettings.GetValue().bStartAfterCreate)
                {
                    UE_LOG_ONLINE_SESSION(Display, TEXT("Session creation completed. Automatic start is turned on, starting session now."));
                    StartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
                    Sessions->StartSession(NAME_GameSession);
                }
                else
                {
                    UE_LOG_ONLINE_SESSION(Display, TEXT("Session creation completed. Automatic start is turned off, to start the session call 'StartSession'."));
                    OnStartSessionSuccess.Broadcast();
                }
            
                return;
            }
        }
    }
    
    if (!bWasSuccessful)
    {
        OnStartSessionFailure.Broadcast();
    }
}

void UEasySessionSubsystem::OnStartSessionComplete(FName SessionName, const bool bWasSuccessful)
{
    const FEasyOnlineHelper Helper(TEXT("StartSessionCallback"), GetWorld());
    
    if (Helper.OnlineSub != nullptr)
    {
        const auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
        }
    }
    
    if (bWasSuccessful)
    {
        OnStartSessionSuccess.Broadcast();
    }
    else
    {
        OnStartSessionFailure.Broadcast();
    }
}

void UEasySessionSubsystem::FindSessions(const FEasySearchSettings& InSettings)
{
    // TODO : AdvancedSessions::FindSessionsCallbackProxyAdvanced 참고
    FEasyOnlineHelper Helper(TEXT("FindSessions"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            FindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

            LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
            LastSessionSearch->MaxSearchResults = InSettings.MaxSearchResults;
            LastSessionSearch->bIsLanQuery = InSettings.bIsLAN;
    
            if (!InSettings.bIsLAN)
            {
                // TODO : AdvancedSessions::FOnlineSeacrhSettingsEx 참고
                LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
            }

            for (const auto& Pair : InSettings.QuerySettings)
            {
                LastSessionSearch->QuerySettings.Set(FName(*Pair.Key), Pair.Value, EOnlineComparisonOp::Equals);
            }
    
            Sessions->FindSessions(*Helper.UserID, LastSessionSearch.ToSharedRef());
            return;
        }
        else
        {
            UE_LOG_ONLINE_SESSION(Display, TEXT("[EasySession] Cannot find sessions: Session Interface is invalid"));
        }
    }
    
    TArray<FOnlineSessionSearchResult> Results;
    OnFindSessionsFailure.Broadcast(Results);
}

void UEasySessionSubsystem::OnFindSessionsComplete(const bool bWasSuccessful)
{
    // TODO : AdvancedSessions::FindSessionsCallbackProxyAdvanced 참고
    FEasyOnlineHelper Helper(TEXT("FindSessionsCallback"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);\
        }
    }
    
    TArray<FOnlineSessionSearchResult> Results;
    if (bWasSuccessful && LastSessionSearch.IsValid())
    {
        Results = LastSessionSearch->SearchResults;
        OnFindSessionsSuccess.Broadcast(Results);
    }
    else
    {
        OnFindSessionsFailure.Broadcast(Results);
    }
}

int32 UEasySessionSubsystem::GetPingInMs(const FOnlineSessionSearchResult& Result)
{
    return Result.PingInMs;
}

FString UEasySessionSubsystem::GetServerName(const FOnlineSessionSearchResult& Result)
{
    return Result.Session.OwningUserName;
}

int32 UEasySessionSubsystem::GetCurrentPlayers(const FOnlineSessionSearchResult& Result)
{
    return Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
}

int32 UEasySessionSubsystem::GetMaxPlayers(const FOnlineSessionSearchResult& Result)
{
    return Result.Session.SessionSettings.NumPublicConnections;
}

void UEasySessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    // TODO : AdvancedSessions에는 JoinSessionCallbackProxyAdvanced가 없다?
    
    FEasyOnlineHelper Helper(TEXT("JoinSession"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            JoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
            Sessions->JoinSession(*Helper.UserID, NAME_GameSession, SessionResult);
            return;
        }
        else
        {
            UE_LOG_ONLINE_SESSION(Display, TEXT("[EasySession] Cannot join session: Session Interface is invalid"));
        }
    }
    
    OnJoinSessionFailure.Broadcast();
}

void UEasySessionSubsystem::OnJoinSessionComplete(FName SessionName, const EOnJoinSessionCompleteResult::Type Result)
{
    FEasyOnlineHelper Helper(TEXT("JoinSessionCallback"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
            
            if (Result == EOnJoinSessionCompleteResult::Success)
            {
                FString ConnectString;
                if (Sessions->GetResolvedConnectString(NAME_GameSession, ConnectString))
                {
                    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
                    {
                        UE_LOG_ONLINE_SESSION(Log, TEXT("Join session: traveling to %s"), *ConnectString);
                        PC->ClientTravel(ConnectString, TRAVEL_Absolute);
                        OnJoinSessionSuccess.Broadcast();
                        return;
                    }
                }
            }
        }
    }
    
    OnJoinSessionFailure.Broadcast();   
}

void UEasySessionSubsystem::DestroySession()
{
    FEasyOnlineHelper Helper(TEXT("DestroySession"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            DestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
            Sessions->DestroySession(NAME_GameSession);
            
            return;
        }
        else
        {
            UE_LOG_ONLINE_SESSION(Display, TEXT("[EasySession] Cannot destroy session: Session Interface is invalid"));
        }
    }
    
    OnDestroySessionFailure.Broadcast();
}

void UEasySessionSubsystem::OnDestroySessionComplete(FName SessionName, const bool bWasSuccessful)
{
    FEasyOnlineHelper Helper(TEXT("DestroySessionCallback"), GetWorld());
    Helper.GetUserID();
    
    if (Helper.IsValid())
    {
        auto Sessions = Helper.OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
            
            if (bWasSuccessful && LastSettings.IsSet())
            {
                CreateSession(LastSettings.GetValue());
                LastSettings.Reset();
            }
        }
    }
    
    if (bWasSuccessful)
    {
        OnDestroySessionSuccess.Broadcast();
    }
    else
    {
        OnDestroySessionFailure.Broadcast();
    }
}

FString UEasySessionSubsystem::GetCurrentSessionProperty(const FString& Key)
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!Subsystem) return FString();

    const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return FString();

    const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!NamedSession) return FString();

    FString Val;
    if (NamedSession->SessionSettings.Get(FName(*Key), Val))
    {
        return Val;
    }

    return FString();
}

bool UEasySessionSubsystem::IsServer() const
{
    if (const UWorld* World = GetWorld())
    {
        return World->GetNetMode() < NM_Client;
    }
    
    return false;
}

bool UEasySessionSubsystem::IsAdmin()
{
    if (IsServer())
    {
        return true;
    }

    const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
    if (Sessions.IsValid())
    {
        const FNamedOnlineSession* NamedSession = Sessions->GetNamedSession(NAME_GameSession);
        if (NamedSession && NamedSession->bHosting)
        {
            return true;
        }
    }

    return false;
}
