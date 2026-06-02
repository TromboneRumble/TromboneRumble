// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultPlayerState.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "Utilities/Defines.h"
#include "TromboneGameInstance.generated.h"

class UAkComponent;
class UAkAudioEvent;

USTRUCT(Blueprintable)
struct FPlayerResultSceneData
{
    GENERATED_BODY()
    
    FString Nickname = FString();
    
    float Score = 0.0f;
    
    FRumbleScoreData SpecificScoreData = FRumbleScoreData();
    
    FLinearColor PlayerSkinColor = FLinearColor::Black;
    
    /** is this my data? */
    bool bIsLocalPlayer = false;
};

UCLASS(Abstract)
class TROMBONERUMBLE_API UTromboneGameInstance : public UGameInstance
{
	GENERATED_BODY()
    
public:
    
    //~ Begin UGameInstance Interface
    virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
    //~ End UGameInstance Interface
    
    /** Save in-game data when travel to the result  */
    void SaveResultSceneData();
    
    UFUNCTION(BlueprintCallable)
    void PlayMenuBGM();
    
    UFUNCTION(BlueprintCallable)
    void StopMenuBGM();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UDataTable> RhythmSongDataTableSoft;
    
    /** 인게임 -> 결과창으로 전환 시 사용할 데이터. 외부에서 Read/Write 가능해야 함 */
    UPROPERTY()
    TArray<FPlayerResultSceneData> CachedResultSceneData;
    
    /** @return My(Local) result scene data (in-game score data)*/
    const FPlayerResultSceneData& GetLocalPlayerResultSceneData();

    /** @return My(Local) rhythm rank */
    int32 GetLocalPlayerRank(); 

protected:
    
    // ~ Begin UGameInstance Interface
    virtual void OnStart() override;
    // ~ End UGameInstance Interface

protected:
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|UI")
    TSoftObjectPtr<UStringTable> CommonStringTable;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config|UI")
    TSoftObjectPtr<UStringTable> TutorialStringTable;
    
private:
    
    void InitWWiseEngine();

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
};
