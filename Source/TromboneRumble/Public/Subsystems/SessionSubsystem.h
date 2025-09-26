// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, const FString&, LobbyCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoinURLReady, const FString&, TravelURL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchFinished, bool, bFoundAny);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, const FString&, Reason);

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSessionWithRandomCode(int32 CodeLength = 6, int32 PublicConnections = 4);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSessionWithCode(const FString& InLobbyCode, int32 PublicConnections = 4);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindAndJoinByCode(const FString& InLobbyCode);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartGameByPath(const FString& InMapPath);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveOrDestroySession();

	// 테스트 시 강제로 LAN 전환(에디터 / Standalone용)
	UFUNCTION(BlueprintCallable, Category = "Session|Test")
	FORCEINLINE void SetForceLANForTesting(bool bEnable) { bForceLANForTesting = bEnable; }

	UFUNCTION(BlueprintPure, Category = "Session")
	FORCEINLINE FString GetCurrentLobbyCode() const { return CurrentLobbyCode; }
public:
	//----------------------------------------Public Variables--------------------------------------------//
	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionJoinURLReady OnSessionJoinURLReady;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionSearchFinished OnSessionSearchFinished;

	UPROPERTY(BlueprintAssignable, Category = "Session|Event")
	FOnSessionError OnSessionError;

private:
	void CreateSession_Internal(const FString& InLobbyCode, int32 PublicConnections);
	void BindDelegates();
	void UnbindDelegates();

	void HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful);

	// 검색 결과 중 코드 일치 항목을 선택
	bool TryChooseResultByCode(const FString& InLobbyCode, FOnlineSessionSearchResult& OutResult) const;

	// TravelURL 계산(조인 후)
	bool TryGetResolvedConnectString(FString& OutURL) const;

	bool IsLanEnvironment() const;
	static FString GenerateRandomCode(int32 Length);
	IOnlineSessionPtr GetSession() const { return SessionInterfaceWeak.Pin(); }
private:
	//----------------------------------------Private Variables--------------------------------------------//
	// Online Subsystem 세션 핸들
	TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> SessionInterfaceWeak;
	// 검색/상태
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateCompleteHandle;
	FDelegateHandle FindCompleteHandle;
	FDelegateHandle JoinCompleteHandle;
	FDelegateHandle DestroyCompleteHandle;

	FString CurrentLobbyCode;

	// 공통 사용하는 세션명
	static const FName SessionName;

	// 커스텀 검색/광고 키 (양쪽 동일키 사용)
	static const FName KEY_LOBBY_CODE;

	UPROPERTY(Transient)
	bool bForceLANForTesting = false;
};
