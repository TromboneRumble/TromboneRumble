// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
    
    UFUNCTION(BlueprintCallable)
    void PlayMenuBGM();
    
    UFUNCTION(BlueprintCallable)
    void StopMenuBGM();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UDataTable> RhythmSongDataTableSoft;

protected:
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|UI")
    TSoftObjectPtr<UStringTable> CommonStringTable;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|UI")
    TSoftObjectPtr<UStringTable> TutorialStringTable;
    
private:
    
    void InitWWiseEngine();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", Categories = "OnlineSession"))
    bool bConnectionWasLost = false;
    
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAkComponent> GlobalBGMComponent;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|BGM")
    TMap<EMenuBGMType, TObjectPtr<UAkAudioEvent>> PlayMenuBGMEvents; 
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|BGM")
    TMap<EMenuBGMType, TObjectPtr<UAkAudioEvent>> StopMenuBGMEvents; 
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", Categories = "Trombone.Rhythm.Song"))
    FGameplayTag SelectedSongTag = FGameplayTag::EmptyTag;
    
    bool bIsMenuMusicPlaying = false;
    EMenuBGMType CurrentMenuBGMType = EMenuBGMType::None;

public:
    // ~ Begin Getter & Setter
    void SetSelectedSongTag(const FGameplayTag& InTag) { SelectedSongTag = InTag; }
    
    FGameplayTag GetSelectedSongTag() const { return SelectedSongTag; }
    
    UFUNCTION(BlueprintCallable, Category = "UI")
    FText GetCommonUIText(const FString& Key) const;
    
    FText GetTutorialUIText(const FString& Key) const;
    
    // ~ End Getter & Setter
    
public:
    
    //~ Begin UGameInstance Interface
    virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
    //~ End UGameInstance Interface
    
protected:
    
    // ~ Begin UGameInstance Interface
    virtual void OnStart() override;
    // ~ End UGameInstance Interface
};
