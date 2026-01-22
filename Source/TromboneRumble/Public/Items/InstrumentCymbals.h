// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InstrumentBase.h"
#include "InstrumentCymbals.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AInstrumentCymbals : public AInstrumentBase
{
	GENERATED_BODY()
public:
	virtual void Multicast_OnHitSuccess_Implementation(AActor* HitActor) override;
	virtual void Client_OnHitSuccess_Implementation(AActor* HitActor) override;
protected:
	virtual float CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo) override;
};
