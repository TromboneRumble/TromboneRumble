// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/RhythmSongDataRow.h"

#if WITH_EDITOR
void FRhythmSongDataRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
    if (!SongTag.IsValid() || InDataTable == nullptr)
    {
        return;
    }

    // RowName == TagName이면 넘어감
    const FName DesiredRowName = SongTag.GetTagName();    
    if (InRowName == DesiredRowName)
    {
        return;
    }

    // 이미 같은 이름의 Row가 있으면 자동 수정 불가 → 경고만 남김
    UDataTable* MutableTable = const_cast<UDataTable*>(InDataTable);
    if (MutableTable->GetRowMap().Contains(DesiredRowName))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[FRhythmSongDataRow] Tag(%s) ↔ RowName(%s) 불일치, RowName을 %s로 바꾸고 싶지만 이미 같은 이름의 Row가 있습니다."),
            *SongTag.ToString(),
            *InRowName.ToString(),
            *DesiredRowName.ToString());
        return;
    }

    // 해당 Row에 있는 데이터를 가지고 새로운 Row 생성 후 기존 Row 삭제
	// 자동 변경 기능은 일단 보류
   /* FRhythmSongDataRow* ThisRow =
        MutableTable->FindRow<FRhythmSongDataRow>(InRowName, TEXT("FRhythmSongDataRow::OnDataTableChanged"));
    if (!ThisRow)
    {
        return;
    }

    FRhythmSongDataRow Copy = *ThisRow;
    MutableTable->RemoveRow(InRowName);
    MutableTable->AddRow(DesiredRowName, Copy);

    UE_LOG(LogTemp, Log,
        TEXT("[FRhythmSongDataRow] RowName 자동 변경: %s → %s (Tag 기준)"),
        *InRowName.ToString(),
        *DesiredRowName.ToString());*/
}
#endif
