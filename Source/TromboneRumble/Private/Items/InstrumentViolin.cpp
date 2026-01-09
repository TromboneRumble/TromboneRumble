// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentViolin.h"
#include "Data/InstrumentScoreData.h"
#include "Utilities/DebugHelper.h"

void AInstrumentViolin::OnRep_Equipped()
{
	Super::OnRep_Equipped();
	if (IsOwnerLocallyControlled())
	{
		TotalNoteCount = 0;
	}
}

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
			BuffRemainingCount = ScoreData->ViolinBuffDurationCount;
		}
	}

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	float GradeMultiplier = GetGradeMultiplier();


	// 등급 점수*배율 + 콤보 점수
	float FinalScore = (BaseScore * GradeMultiplier) + ScoreData->ComboBasePoint;

	if (IsOwnerLocallyControlled())
	{
		FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		FString DebugMsg = FString::Printf(
			TEXT("[Violin] Result: %s | (Base: %.0f * Mult:%.1f) + ComboBonus: %.0f = Final: %.0f"),
			*NoteResultStr,
			BaseScore,
			GradeMultiplier,
			ScoreData->ComboBasePoint,
			FinalScore
		);

		Debug::Print(DebugMsg);
	}


	return FinalScore;
}
