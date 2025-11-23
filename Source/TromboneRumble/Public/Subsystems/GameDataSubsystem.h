// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameDataSubsystem.generated.h"

struct FRhythmSongDataRow;
/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UGameDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    UPROPERTY()
    UDataTable* RhythmSongDataTable = nullptr;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    void PreloadSongAssets(const FGameplayTag& InSongTag);
    const FRhythmSongDataRow* GetSongRow(const FGameplayTag& InSongTag) const;

};
