// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TromboneGamePlayTags.h"
#include "Engine/GameInstance.h"
#include "Utilities/Defines.h"
#include "TromboneGameInstance.generated.h"

class UAkComponent;
class UAkAudioEvent;

UCLASS(Abstract)
class TROMBONERUMBLE_API UTromboneGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
    virtual void OnStart() override;
    
    UFUNCTION(BlueprintCallable)
    void PlayMenuBGM();
    
    UFUNCTION(BlueprintCallable)
    void StopMenuBGM();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UDataTable> RhythmSongDataTableSoft;

private:
    void InitWWiseEngine();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAkComponent> GlobalBGMComponent;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|BGM")
    TMap<EMenuBGMType, TObjectPtr<UAkAudioEvent>> PlayMenuBGMEvents; 
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|BGM")
    TMap<EMenuBGMType, TObjectPtr<UAkAudioEvent>> StopMenuBGMEvents; 
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", Categories = "Trombone.Rhythm.Song"))
    FGameplayTag SelectedSongTag = TromboneGamePlayTags::Trombone_Rhythm_Song_MapA;
    
    int32 SessionPlayerNumber = 2;
    bool bIsMenuMusicPlaying = false;
    EMenuBGMType CurrentMenuBGMType = EMenuBGMType::None;

public:
    // ~ Begin Getter & Setter
    void SetSelectedSongTag(const FGameplayTag& InTag) { SelectedSongTag = InTag; }
    FGameplayTag GetSelectedSongTag() const { return SelectedSongTag; }
    
    void SetSessionPlayerNumber(const int32 InNumber) { SessionPlayerNumber = InNumber; }
    int32 GetSessionPlayerNumber() const { return SessionPlayerNumber; }
    // ~ End Getter & Setter
};
