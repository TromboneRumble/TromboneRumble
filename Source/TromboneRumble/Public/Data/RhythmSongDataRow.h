// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Utilities/Defines.h"
#include "TromboneGamePlayTags.h"
#include "RhythmSongDataRow.generated.h"

class UAkAudioEvent;
class UAkSwitchValue;

USTRUCT(BlueprintType)
struct FRhythmInstrumentSound
{
    GENERATED_BODY()

    // 어떤 악기인지
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EInstrumentType InstrumentType = EInstrumentType::Trombone;

    // 노트 성공 시 재생할 이벤트
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAkAudioEvent> NoteEvent;

    // 롱노트 change 시에 바꿀 Switch
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAkSwitchValue> ChangeSwitch;

    // 실패 시 재생할 이벤트
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAkAudioEvent> FailEvent;
};

USTRUCT(BlueprintType)
struct FRhythmSongDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // 이 Row를 식별할 GameplayTag (키 역할)
    // RowName이랑 같은 값으로 맞춰두면 나중에 찾기 쉬움.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Trombone.Rhythm.Song"))
    FGameplayTag SongTag;

    // 곡 BGM 이벤트 (비워도 됨)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAkAudioEvent> BgmEvent;

    // "None" 상태로 쓸 Switch (비워도 됨)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAkSwitchValue> NoneSwitch;

    // 이 곡에서 사용할 악기별 사운드 설정들
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FRhythmInstrumentSound> InstrumentSounds;
};