// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/WorldInitializationValues.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Utilities/Defines.h"
#include "GameplayTagContainer.h"
#include "GameStateSubsystem.generated.h"

struct FGameplayTag;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStateChangedSignature, ELevelState, NewState);
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
	UFUNCTION()
	void OnPostLoadMap(UWorld* InLoadedWorld);

	void SetLevelState(const ELevelState& InNewState);


private:
	void AddMapPathFromGameTag(const FGameplayTag& InTag, const ELevelState& InLevelState);

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ELevelState CurrentLevelState;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FString> MapTagToMapNameMap;

	UPROPERTY(Transient)
	TMap<FGameplayTag, ELevelState> MapTagToLevelStateMap;
public:
	// Getter Setter
	FString GetMapNameForTag(const FGameplayTag& MapTag) const;
	FORCEINLINE ELevelState GetLevelState() const { return CurrentLevelState; }
};
