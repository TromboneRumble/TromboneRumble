// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/WorldInitializationValues.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Utilities/Defines.h"
#include "GameplayTagContainer.h"
#include "GameStateSubsystem.generated.h"

struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStateChangedSignature, ELevelType, NewState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLoadingScreenFinishedSignature, APlayerController*);

UCLASS()
class TROMBONERUMBLE_API UGameStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnLevelStateChangedSignature OnLevelStateChanged;
	FOnPlayerLoadingScreenFinishedSignature OnPlayerLoadingScreenFinished;

protected:
	
	void OnPreLoadMap(const FString& InMapName);
	void OnPostLoadMap(UWorld* InLoadedWorld);

	void SetLevelState(const ELevelType& InNewState);

private:
	
	void AddMapPathFromGameTag(const FGameplayTag& InTag, const ELevelType& InLevelState);
	
	bool UpdateLevelStateFromMapName(const FString& InMapName);

	ELevelType CurrentLevelType;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FString> MapTagToMapNameMap;

	UPROPERTY(Transient)
	TMap<FGameplayTag, ELevelType> MapTagToLevelTypeMap;
	
public:
	
	// Getter Setter
	FString GetLevelStringFromTag(const FGameplayTag& MapTag) const;
	ELevelType GetLevelState() const;
	
	void DumpSettings() const;
};
