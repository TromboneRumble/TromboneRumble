// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TromboneGamePlayTags.h"
#include "Engine/GameInstance.h"
#include "TromboneGameInstance.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API UTromboneGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UDataTable> RhythmSongDataTableSoft;

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", Categories = "Trombone.Rhythm.Song"))
    FGameplayTag SelectedSongTag = TromboneGamePlayTags::Trombone_Rhythm_Song_MapA;

public:
    //getter setter
    void SetSelectedSongTag(const FGameplayTag& InTag) { SelectedSongTag = InTag; }
    FGameplayTag GetSelectedSongTag() const { return SelectedSongTag; }
};
