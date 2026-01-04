// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentTrombone.h"
#include "Data/InstrumentScoreData.h"
#include "Utilities/DebugHelper.h"

float AInstrumentTrombone::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	// --- 버프 관리 로직 ---
	if (CurrentCombo >= ScoreData->TromboneBuffComboThreshold)
	{
		// 조건 만족 시 버프 적용 (이미 있으면 ApplyBuff 내부에서 무시됨)
		ApplyBuff(ScoreData->TromboneBuffEffectClass);
	}
	else
	{
		// 조건 불만족 시 버프 해제
		RemoveBuff();
	}

	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None) return 0.f;

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	float ComboMultiplier = GetComboMultiplier();

	// 등급 점수 + (콤보 점수 * 배율)
	float FinalScore = BaseScore + (ScoreData->ComboBasePoint * ComboMultiplier);

	if (IsOwnerLocallyControlled())
	{
		FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		// 예: [Trombone] Result: Excellent | Base: 100 + (ComboBonus: 10 * Mult: 2.0) = Final: 120
		FString DebugMsg = FString::Printf(
			TEXT("[Trombone] Result: %s | Base: %.0f + (ComboBonus: %.0f * Mult: %.1f) = Final: %.0f"),
			*NoteResultStr,
			BaseScore,
			ScoreData->ComboBasePoint,
			ComboMultiplier,
			FinalScore
		);

		// 요청하신 함수 시그니처에 맞춰 호출
		Debug::Print(DebugMsg);
	}
	
	return FinalScore;
}
