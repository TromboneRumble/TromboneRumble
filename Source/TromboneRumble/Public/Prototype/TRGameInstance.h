// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "TRGameInstance.generated.h"

#define CURRENT_CONTEXT *FString(__FUNCTION__)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FString&, LobbyCode);

UCLASS()
class TROMBONERUMBLE_API UTRGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	void HostSessionWithCode();
	void FindAndJoinSessionByCode(const FString& SessionCode);

	UPROPERTY()
	FOnSessionJoined OnSessionJoined;

protected:
	virtual void Init() override;

private:
	FString GenerateRandomCode(int32 Length);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FString CurrentLobbyCode;
};
