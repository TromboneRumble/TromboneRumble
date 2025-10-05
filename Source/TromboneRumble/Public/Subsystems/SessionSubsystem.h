// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreateComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionsFindComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionJoinComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionDestroyComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStartComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListUpdated, const TArray<FString>&, PlayerNames);

struct FRecreateSessionRequest
{
	int32 NumPublicConnections;
	FString LobbyCode;
	
	FRecreateSessionRequest(const int32 InNumPublicConnections, const FString& InLobbyCode) : NumPublicConnections(InNumPublicConnections), LobbyCode(InLobbyCode) { }
};

UCLASS()
class TROMBONERUMBLE_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USessionSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CreateSession(int32 NumPublicConnections, const FString& LobbyCode);
	void FindSessions(int32 MaxSearchResults, const FString& InLobbyCode = FString(TEXT("")));
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();

	bool TryGetCurrentLobbyCode(FString& OutLobbyCode);

	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsLocalHost() const;

	bool IsLanEnvironment() const;

	bool FindMatchingLobbyInResult(const FString& InLobbyCode, FOnlineSessionSearchResult& OutResult) const;

public:
	//----------------------------------------Public Variables--------------------------------------------//

	// 외부 UI에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionCreateComplete OnSessionCreateComplete;

	FOnSessionsFindComplete OnSessionSearchFinished;

	FOnSessionJoinComplete OnSessionJoinComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionDestroyComplete OnSessionDestroyComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionError OnSessionError;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionStartComplete OnSessionStart;

	UPROPERTY()
	FOnPlayerListUpdated OnPlayerListUpdated;

	// 커스텀 검색/광고 키 (양쪽 동일키 사용)
	static const FName KEY_LOBBY_CODE;

private:
	void HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleNetworkFailure(UWorld* InWorld, UNetDriver* InNetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType, const FString& ErrorString);

	bool IsValidSessionInterface();

private:
	//----------------------------------------Private Variables--------------------------------------------//
	// Online Subsystem 세션 핸들
	TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> SessionInterfaceWeak;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	// Online Subsystem delegate 핸들
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	TOptional<FRecreateSessionRequest> RecreateSessionRequest;
};
