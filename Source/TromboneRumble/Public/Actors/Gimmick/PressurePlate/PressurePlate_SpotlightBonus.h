// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "PressurePlate_SpotlightBonus.generated.h"

UCLASS()
class TROMBONERUMBLE_API APressurePlate_SpotlightBonus : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	APressurePlate_SpotlightBonus();
protected:
	virtual void BeginPlay() override;
	
	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	// ~ End IInteractable Interfaces

	void SpawnSpotlights();
};
