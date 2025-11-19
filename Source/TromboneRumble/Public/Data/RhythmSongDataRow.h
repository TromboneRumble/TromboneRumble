// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Utilities/Defines.h"
#include "TromboneGamePlayTags.h"
#include "RhythmSongDataRow.generated.h"

class AInstrumentBase;
class UAkAudioEvent;
class UAkSwitchValue;

USTRUCT(BlueprintType)
struct FRhythmInstrumentSound
{
    GENERATED_BODY()

   
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "어떤 악기인지"
        ))
    EInstrumentType InstrumentType = EInstrumentType::Invalid;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "악기에 해당하는 Actor"
        ))
    TSubclassOf<AInstrumentBase> SpawnInstrument;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        ToolTip = "노트 성공 시 재생할 이벤트"
        ))
    TSoftObjectPtr<UAkAudioEvent> NoteEvent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "악기를 들었을때 변경할 Switch"
        ))
    TSoftObjectPtr<UAkSwitchValue> ChangeSwitch;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "노트 실패 시 재생할 이벤트"
        ))
    TSoftObjectPtr<UAkAudioEvent> FailEvent;
};

USTRUCT(BlueprintType)
struct FRhythmSongDataRow : public FTableRowBase
{
    GENERATED_BODY()

    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        Categories = "Trombone.Rhythm.Song",
		ToolTip = "해당 Row의 이름을 꼭 Tag랑 맞출것"
        ))
    FGameplayTag SongTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        ToolTip = "인게임에서 보여질 곡의 이름"
        ))
    FString SongName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
        ToolTip = "곡의 총 길이"
        ))
    float SongLength;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "이 곡의 BGM으로 재생할 AkAudioEvent"
        ))
    TSoftObjectPtr<UAkAudioEvent> BgmEvent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "악기를 떨궜을때 재생할 Switch"
        ))
    TSoftObjectPtr<UAkSwitchValue> NoneSwitch;

  
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (
		ToolTip = "이 곡에서 사용할 악기별 사운드 설정들"
        ))
    TArray<FRhythmInstrumentSound> InstrumentSounds;
};