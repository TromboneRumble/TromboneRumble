// Fill out your copyright notice in the Description page of Project Settings.


#include "InstrumentCymbals.h"
#include "Data/InstrumentScoreData.h"
#include "Framework/DefaultPlayerState.h"
#include "Utilities/DebugHelper.h"

void AInstrumentCymbals::OnHitSuccess(AActor* HitActor)
{
	Super::OnHitSuccess(HitActor);
	if (!IsOwnerLocallyControlled()) return;

	PlayHitSound();
	
	if (ADefaultPlayerState* DefaultPlayerState = GetOwnerPlayerState())
	{
		if (IsOwnerLocallyControlled())
		{
			FString DebugMsg = FString::Printf(
				TEXT("[Cymbals] HitResult: HitScore: %.0f"),
				ScoreData->AttackScore
			);
			Debug::Print(DebugMsg);
		}

		DefaultPlayerState->Server_AddScore(FMath::RoundToInt(ScoreData->AttackScore));
	}
}

float AInstrumentCymbals::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None) return 0.f;

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;

	// 등급 점수 + 콤보 점수
	float FinalScore = BaseScore + ScoreData->ComboBasePoint;

	if (IsOwnerLocallyControlled())
	{
		FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		FString DebugMsg = FString::Printf(
			TEXT("[Cymbals] Result: %s | Base: %.0f + ComboBonus: %.0f = Final: %.0f"),
			*NoteResultStr,
			BaseScore,
			ScoreData->ComboBasePoint,
			FinalScore
		);
		Debug::Print(DebugMsg);
	}


	return FinalScore;
}
