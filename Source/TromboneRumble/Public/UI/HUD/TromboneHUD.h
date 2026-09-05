// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TromboneHUD.generated.h"

class UCommonActivatableWidget;

/**
 * Pushes this level's screen onto the shared root UI. Spawned per level by the game mode, owns no widget itself.
 *
 * @see UTromboneUISubsystem
 * @see URootUI
 */
UCLASS()
class TROMBONERUMBLE_API ATromboneHUD : public AHUD
{
	GENERATED_BODY()

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

private:

	/** Screen pushed onto the Base stack while this level is active. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UCommonActivatableWidget> ScreenClass;

	/** Should the FPS and ping overlay be shown in this level. The root keeps the last value, so every level sets it. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	bool bShowPerformanceWidget = false;
};
