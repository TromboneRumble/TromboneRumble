// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InstrumentBase.h"
#include "InstrumentTrombone.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentTrombone : public AInstrumentBase
{
	GENERATED_BODY()

public:
	virtual void OnHitSuccess(AActor* HitActor) override;
	
protected:
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) override;
};
