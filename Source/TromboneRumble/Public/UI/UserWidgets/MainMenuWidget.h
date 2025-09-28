// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenuWidget.generated.h"


class UButton;
class USessionSubsystem;
class UEditableText;

// TODO : 추후 팀전 고려.
UENUM(BlueprintType)
enum class MatchState : uint8
{
	FreeForAll,
	TwoTeams,
	Invalid UMETA(Hidden)
};

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Blueprint PreConstruct에서 호출됨.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void InitSettings(const int32 InNumPublicConnections, const int32 InMaxLobbyCodeLength, const MatchState InState);

protected:
	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:

	// SessionSubsystem Callbacks
	void BindSubsystemCallbacks();
	void RemoveSubsystemCallbacks();

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnSessionError(const FString& Reason);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	// ~ SessionSubsystem Callbacks

	// Button Callbacks
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();
	// ~ Button Callbacks

	FString GenerateRandomLobbyCode(int32 Length);
	const TCHAR* JoinSessionResultToText(const EOnJoinSessionCompleteResult::Type InResult) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> LobbyCodeText;


	int32 NumPublicConnections{ 4 };
	int32 MaxLobbyCodeLength{ 5 };
	MatchState State{ MatchState::Invalid };

	UPROPERTY(Transient)
	TObjectPtr<USessionSubsystem> SessionsSubsystem;

	UPROPERTY(Transient)
	FString CachedLobbyMapPath{ TEXT("") };
};
