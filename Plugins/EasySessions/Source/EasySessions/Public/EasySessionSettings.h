#pragma once

#include "CoreMinimal.h"
#include "EasySessionSettings.generated.h"

static FName GKey_Lobby_Code = FName("LOBBY_CODE");

USTRUCT()
struct FEasySessionSettings
{
	GENERATED_BODY()

	int32 NumPublicConnections = 4;
	int32 NumPrivateConnections = 0;
	
	bool bIsLAN = false;
	bool bIsDedicated = false;
	bool bShouldAdvertise = true;
	bool bAllowJoinInProgress = true;
	bool bAllowJoinViaPresence = true;
	bool bStartAfterCreate = true;
	bool bAllowJoinViaPresenceFriendsOnly = false;
	bool bUsesStats = true;
	bool bUseLobbiesIfAvailable = true;
	bool bAllowInvites = true;
	bool bUseLobbiesVoiceChatIfAvailable = false;
	bool bAntiCheatProtected = false;

	TMap<FString, FString> CustomProperties;

	FEasySessionSettings() {}
	FEasySessionSettings(int32 InMaxPlayers, bool bInIsLAN) 
		: NumPublicConnections(InMaxPlayers), bIsLAN(bInIsLAN) {}
};

USTRUCT()
struct FEasySearchSettings
{
	GENERATED_BODY()

	int32 MaxSearchResults = 100;
	bool bIsLAN = false;
    
	TMap<FString, FString> QuerySettings;

	FEasySearchSettings() {}
	FEasySearchSettings(bool bInIsLAN) : bIsLAN(bInIsLAN) {}
};

DECLARE_MULTICAST_DELEGATE(FOnStartSessionSuccess);
DECLARE_MULTICAST_DELEGATE(FOnStartSessionFailure);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsSuccess, const TArray<FOnlineSessionSearchResult>& /*Results*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsFailure, const TArray<FOnlineSessionSearchResult>& /*Results*/);

DECLARE_MULTICAST_DELEGATE(FOnJoinSessionSuccess);
DECLARE_MULTICAST_DELEGATE(FOnJoinSessionFailure);

DECLARE_MULTICAST_DELEGATE(FOnDestroySessionSuccess);
DECLARE_MULTICAST_DELEGATE(FOnDestroySessionFailure);