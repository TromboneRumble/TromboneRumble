// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/WorldInitializationValues.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Utilities/Defines.h"
#include "GameStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, EGameState, NewState);

UCLASS()
class TROMBONERUMBLE_API UGameStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FORCEINLINE EGameState GetGameState() const { return CurrentGameState; }

	FOnGameStateChangedSignature OnGameStateChanged;
	
protected:
	UFUNCTION()
	void OnPostLoadMap(UWorld* LoadedWorld);
	
	void SetGameState(EGameState NewState);

	EGameState CurrentGameState;
	FString CachedMainMenuMapName;
	FString CachedLobbyMapName;
	FString CachedInGameMapName;
};
