// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasySessionSettings.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EasySessionSubsystem.generated.h"

struct FEasySessionSettings;

UCLASS()
class EASYSESSIONS_API UEasySessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UEasySessionSubsystem();
	
	void CreateSession(const FEasySessionSettings& InSettings);
	void FindSessions(const FEasySearchSettings& InSettings);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	
	FOnStartSessionSuccess OnStartSessionSuccess;
	FOnStartSessionFailure OnStartSessionFailure;
	
	FOnFindSessionsSuccess OnFindSessionsSuccess;
	FOnFindSessionsFailure OnFindSessionsFailure;
	
	FOnJoinSessionSuccess OnJoinSessionSuccess;
	FOnJoinSessionFailure OnJoinSessionFailure;
	
	FOnDestroySessionSuccess OnDestroySessionSuccess;
	FOnDestroySessionFailure OnDestroySessionFailure;
	
	static int32 GetPingInMs(const FOnlineSessionSearchResult& Result);
	static FString GetServerName(const FOnlineSessionSearchResult& Result);
	static int32 GetCurrentPlayers(const FOnlineSessionSearchResult& Result);
	static int32 GetMaxPlayers(const FOnlineSessionSearchResult& Result);

public:
	FString GetCurrentSessionProperty(const FString& Key);
	bool IsServer() const;
	bool IsAdmin();

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// The delegate handles to unregister delegates
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	// The delegate executed by the online subsystem
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	
	TOptional<FEasySessionSettings> LastSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
};