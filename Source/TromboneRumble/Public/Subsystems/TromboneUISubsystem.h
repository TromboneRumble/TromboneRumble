// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TromboneUISubsystem.generated.h"

class URootUI;

/**
 * Owns the single root layout for the local player.
 * The root lives as long as the game instance. Each level only attaches it to the new player controller.
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Get the UI subsystem */
	UFUNCTION(BlueprintPure, Category = "Trombone|Subsystem", DisplayName = "Get UI Subsystem", meta = (WorldContext = "WorldContextObject"))
	static UTromboneUISubsystem* Get(const UObject* WorldContextObject);

	/** @return The root layout. Null until the first player controller arrives */
	URootUI* GetRootUI() const { return RootUI; }

	/**
	 * Creates the root on first call, then attaches it to the given player controller.
	 * Called by ATromboneHUD::BeginPlay. Does nothing when the root already belongs to this controller.
	 */
	void AttachRootUI(APlayerController* PlayerController);

private:

	UPROPERTY(Transient)
	TObjectPtr<URootUI> RootUI;

public:

	//~ Begin USubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;
	//~ End USubsystem interface
};
