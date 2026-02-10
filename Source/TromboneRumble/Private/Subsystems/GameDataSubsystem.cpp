// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameDataSubsystem.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"
#include "AkAudioEvent.h"
#include "AkSwitchValue.h"

void UGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
    {
        if (GI->RhythmSongDataTableSoft.IsNull() == false)
        {
            RhythmSongDataTable = GI->RhythmSongDataTableSoft.LoadSynchronous();

            if (!RhythmSongDataTable)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to load RhythmSongDataTable from SoftObjectPtr"));
            }
        }
    }
}

const FRhythmSongDataRow* UGameDataSubsystem::GetSongRow(const FGameplayTag& InSongTag) const
{
    if (!RhythmSongDataTable || !InSongTag.IsValid()) return nullptr;

    static const FString Ctx = TEXT("GetSongRow");
    return RhythmSongDataTable->FindRow<FRhythmSongDataRow>(InSongTag.GetTagName(), Ctx, true);
}

void UGameDataSubsystem::HandleMusicCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
    if (CallbackType == EAkCallbackType::Duration)
    {
        if (UAkDurationCallbackInfo* DurationCallbackInfo = Cast<UAkDurationCallbackInfo>(CallbackInfo))
        {
            if (DurationCallbackInfo->Duration > 0.f)
            {
                CurrentSongTotalLength = DurationCallbackInfo->Duration / 1000.f;
            }
        }
    }
}

void UGameDataSubsystem::PreloadSongAssets(const FGameplayTag& InSongTag)
{
    const FRhythmSongDataRow* Row = GetSongRow(InSongTag);
    if (!Row) return;

    // BGM
    Row->BgmEvent.LoadSynchronous();
    Row->NoneSwitch.LoadSynchronous();

    // 악기 사운드
    for (auto& Sound : Row->InstrumentSounds)
    {
        Sound.NoteEvent.LoadSynchronous();
        Sound.ChangeSwitch.LoadSynchronous();
        Sound.FailEvent.LoadSynchronous();
    }
}
