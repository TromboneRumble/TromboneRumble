// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "GameFramework/Pawn.h"
#include "TutorialDummy.generated.h"

class ATutorialManager;

UCLASS()
class TROMBONERUMBLE_API ATutorialDummy : public ADefaultTromboneCharacter
{
	GENERATED_BODY()
	
public:
	// ~ Begin ICombatReceiver Interfaces
	virtual void OnHitReceived_Implementation(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces

private:
	
	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
};