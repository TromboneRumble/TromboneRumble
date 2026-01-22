// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentTrombone.h"
#include "Data/InstrumentScoreData.h"
#include "UI/UserWidgets/Rhythm/ComboWidget/TromboneComboWidget.h"
#include "Utilities/DebugHelper.h"

void AInstrumentTrombone::Multicast_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Multicast_OnHitSuccess_Implementation(HitActor);
	PlayHitSound();
}

float AInstrumentTrombone::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	
	if (!ComboWidgetInstance.Get()) return 0.f;
	UTromboneComboWidget* TromboneComboWidget = Cast<UTromboneComboWidget>(ComboWidgetInstance.Get());
	
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None)
	{
		if (ActiveBuffHandle.IsValid())
		{
			RemoveBuff(CurrentOwner);
		}
		TromboneComboWidget->SetPercentSmooth(0.f);
		return 0.f;
	}
	else
	{
		if (CurrentCombo>=ScoreData->TromboneBuffComboThreshold && !ActiveBuffHandle.IsValid())
		{
			ApplyBuff(ScoreData->TromboneBuffEffectClass);
		}
		TromboneComboWidget->SetPercentSmooth(FMath::Min(1.0f, (float)CurrentCombo / (float)ScoreData->TromboneBuffComboThreshold));
	}

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

		Debug::Print(DebugMsg);
	}
	
	return FinalScore;
}
