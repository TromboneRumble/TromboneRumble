// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentViolin.h"
#include "Data/InstrumentScoreData.h"

float AInstrumentViolin::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None) return 0.f;

	TotalNoteCount++;

	// --- 버프 관리 로직 ---
	if (ActiveBuffHandle.IsValid())
	{
		// 이미 버프 중이라면 횟수 차감
		BuffRemainingCount--;
		if (BuffRemainingCount <= 0)
		{
			RemoveBuff(); // 횟수 소진 시 버프 해제
		}
	}
	else
	{
		// 버프가 없는 상태에서 발동 조건 확인 (N번째 노트마다)
		if (TotalNoteCount > 0 && (TotalNoteCount % ScoreData->ViolinBuffActivationCount == 0))
		{
			ApplyBuff(ScoreData->ViolinBuffEffectClass);
			BuffRemainingCount = ScoreData->ViolinBuffActivationCount;
		}
	}

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	float Multiplier = GetGradeMultiplier();

	return (BaseScore * Multiplier) + ScoreData->ComboBasePoint;
}
