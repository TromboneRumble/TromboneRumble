// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/WorldInitializationValues.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Utilities/Defines.h"
#include "GameplayTagContainer.h"
#include "GameStateSubsystem.generated.h"

struct FGameplayTag;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, EGameState, NewState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLoadingScreenFinishedSignature, APlayerController*);

UCLASS()
class TROMBONERUMBLE_API UGameStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnGameStateChangedSignature OnGameStateChanged;
	FOnPlayerLoadingScreenFinishedSignature OnPlayerLoadingScreenFinished;

protected:
	UFUNCTION()
	void OnPostLoadMap(UWorld* InLoadedWorld);

	void SetGameState(const EGameState& InNewState);


private:
	void AddMapPathFromGameTag(const FGameplayTag& InTag, const EGameState& InGameState);

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EGameState CurrentGameState;

	UPROPERTY(Transient)
	TMap<EGameState, FString> GameStateToMapNameMap;
public:
	// Getter Setter
	FString GetMapNameForGameState(const EGameState& InGameState) const;
	FORCEINLINE EGameState GetGameState() const { return CurrentGameState; }
};
