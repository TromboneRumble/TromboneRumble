// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameDataSubsystem.generated.h"

class UAkCallbackInfo;
enum class EAkCallbackType : uint8;
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

    UFUNCTION()
    void HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);

protected:
    int32 CurrentSongPlayingID = 0;
    float CurrentSongTotalLength = 0.f;


public:
    FORCEINLINE int32 GetCurrentSongPlayingID() const { return CurrentSongPlayingID; }
    FORCEINLINE void SetCurrentSongPlayingID(int32 InPlayingID) { CurrentSongPlayingID = InPlayingID; }
    FORCEINLINE float GetCurrentSongLength() const { return CurrentSongTotalLength; }

};
