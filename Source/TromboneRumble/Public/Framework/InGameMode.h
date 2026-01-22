// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/ItemEquipHandler.h"
#include "InGameMode.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGameMode : public AGameModeBase, public IItemEquipHandler
{
	GENERATED_BODY()

public:
	AInGameMode();
	
	// IInstrumentEquipHandler Interfaces
	virtual void HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem) override;
	virtual void HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem) override;
	// ~IInstrumentEquipHandler Interfaces

	virtual void BeginPlay() override;
	
	void GameEnd() const;
	void OnRhythmGameEndedReport();

	UFUNCTION()
	void HandlePlayerLoadingFinished(APlayerController* PC);

private:
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> InGameReadyPlayers;
	int32 SessionPlayerNumber = 1;
	int32 RhythmGameEndedPlayerCount = 0;
};
