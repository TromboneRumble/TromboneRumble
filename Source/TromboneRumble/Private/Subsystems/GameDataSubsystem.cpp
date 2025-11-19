// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameDataSubsystem.h"
#include "Data/RhythmSongDataRow.h"
#include "Framework/TromboneGameInstance.h"

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

const FRhythmSongDataRow* UGameDataSubsystem::GetSongRow(const FGameplayTag& SongTag) const
{
    if (!RhythmSongDataTable || !SongTag.IsValid()) return nullptr;

    static const FString Ctx = TEXT("GetSongRow");
    return RhythmSongDataTable->FindRow<FRhythmSongDataRow>(SongTag.GetTagName(), Ctx, true);
}
