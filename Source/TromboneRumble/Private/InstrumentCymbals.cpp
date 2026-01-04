// Fill out your copyright notice in the Description page of Project Settings.


#include "InstrumentCymbals.h"
#include "Data/InstrumentScoreData.h"
#include "Framework/DefaultPlayerState.h"

void AInstrumentCymbals::OnHitSuccess(AActor* HitActor)
{
	Super::OnHitSuccess(HitActor);
	if (!IsOwnerLocallyControlled()) return;

	// 타격 점수 서버로 전송
	if (ADefaultPlayerState* DefaultPlayerState = GetOwnerPlayerState())
	{
		DefaultPlayerState->Server_AddScore(FMath::RoundToInt(ScoreData->AttackScore));
	}
}

float AInstrumentCymbals::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None) return 0.f;

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	return BaseScore + ScoreData->ComboBasePoint;
}
