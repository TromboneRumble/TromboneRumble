// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "PressurePlate_SpawnPresent.generated.h"

class APresent;

UCLASS()
class TROMBONERUMBLE_API APressurePlate_SpawnPresent : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APressurePlate_SpawnPresent();

protected:
	virtual void BeginPlay() override;

	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	// ~ End IInteractable Interfaces

	void SpawnPresent();

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick", meta = (DisplayName = "선물 클래스"))
	TSubclassOf<APresent> PresentClass;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick", meta = (DisplayName = "Z 오프셋 (cm)", ClampMin = "0.0"))
	float SpawnZOffset = 500.f;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick", meta = (DisplayName = "선물 획득 점수"))
	int32 PresentBonusScore = 300;
};
