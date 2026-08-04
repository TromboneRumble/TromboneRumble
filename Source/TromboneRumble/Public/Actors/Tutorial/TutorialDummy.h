// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "GameFramework/Pawn.h"
#include "TutorialDummy.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATutorialDummy : public ADefaultTromboneCharacter
{
	GENERATED_BODY()
	
public:

	ATutorialDummy();

	// ~ Begin ICombatReceiver Interfaces
	virtual bool OnHitReceived_Implementation(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces

};