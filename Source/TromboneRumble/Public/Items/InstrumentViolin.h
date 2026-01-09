// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InstrumentBase.h"
#include "InstrumentViolin.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentViolin : public AInstrumentBase
{
	GENERATED_BODY()
protected:
	virtual void OnRep_Equipped() override;
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) override;
private:
	int32 TotalNoteCount = 0;       // 전체 누적 노트
	int32 BuffRemainingCount = 0;   // 버프가 유지될 남은 노트 수
};
