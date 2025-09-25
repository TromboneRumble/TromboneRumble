// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_TRGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "Utilities/DebugHelper.h"

void UPT_TRGameInstance::Init()
{
    Super::Init();
    
    const IOnlineSubsystem* OnlineSubsystemInterface = Online::GetSubsystem(GetWorld());
    if (nullptr == OnlineSubsystemInterface)
    {
       if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No Online Subsystem found!"));
       return;
    }
    
    SessionInterface = OnlineSubsystemInterface->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
       SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPT_TRGameInstance::OnCreateSessionComplete);
       SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPT_TRGameInstance::OnFindSessionComplete);
       SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPT_TRGameInstance::OnJoinSessionComplete);
    }
}

FString UPT_TRGameInstance::GenerateRandomCode(int32 Length)
{
    const FString Chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    FString RandomCode;
    for (int32 i = 0; i < Length; ++i)
    {
       RandomCode += Chars[FMath::RandRange(0, Chars.Len() - 1)];
    }
    return RandomCode;
}

void UPT_TRGameInstance::HostSessionWithCode()
{
   if (!SessionInterface.IsValid())
   {
       Debug::Print(TEXT("SessionInterface not Valid"));
       return;
   }

   CurrentLobbyCode = GenerateRandomCode(5);

   const TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
   SessionSettings->bIsDedicated = false;
   SessionSettings->bAllowJoinViaPresence = true;
   SessionSettings->bIsLANMatch = false;
   SessionSettings->bShouldAdvertise = true;
   SessionSettings->NumPublicConnections = 4;
   SessionSettings->bUsesPresence = true;
   SessionSettings->bUseLobbiesIfAvailable = true;

   SessionSettings->Set(FName("LOBBY_CODE_KEY"), CurrentLobbyCode, EOnlineDataAdvertisementType::ViaOnlineService);
   FString MapName = GetWorld()->GetMapName();
   MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
   SessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
   
   const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

   if (SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
   {
      if (GEngine)
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Create Session Success"));
   }
   else
   {
       Debug::Print("HostSessionWithCode.CreateSession Failed");
   }
}

void UPT_TRGameInstance::FindAndJoinSessionByCode(const FString& SessionCode)
{
    if (!SessionInterface.IsValid() || SessionCode.IsEmpty()) return;

    SessionSearch = MakeShared<FOnlineSessionSearch>();
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 1;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionSearch->QuerySettings.Set(FName("LOBBY_CODE_KEY"), SessionCode, EOnlineComparisonOp::Equals);
    
    const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
    SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
    
    if (GEngine)
       GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, FString::Printf(TEXT("Searching for lobby with code: %s"), *SessionCode));
}

void UPT_TRGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
   if (bWasSuccessful)
   {
      if (GEngine)
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Session created successfully: %s"), *SessionName.ToString()));

      OnSessionJoined.Broadcast(CurrentLobbyCode);
   }
   else
   {
      if (GEngine)
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Failed to create session."));
   }
}

void UPT_TRGameInstance::OnFindSessionComplete(const bool bWasSuccessful)
{
    if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
       if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Lobby found!"));
       FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[0];
       SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
       SearchResult.Session.SessionSettings.bUsesPresence = true;
        
       const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
       SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult);
    }
    else
    {
       if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Could not find lobby."));
    }
}

void UPT_TRGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
   if (Result == EOnJoinSessionCompleteResult::Success)
   {
      if (GEngine)
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Joined session complete."));
    
      FString ConnectionAddress;
      if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectionAddress))
      {
         OnSessionJoined.Broadcast(CurrentLobbyCode);
         GetWorld()->GetFirstPlayerController()->ClientTravel(ConnectionAddress, ETravelType::TRAVEL_Absolute);
      }
   }
   else
   {
      if (GEngine)
      {
         GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Failed to join session. Result: %s"), LexToString(Result)));
      }
   }
}