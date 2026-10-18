// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Utilities/Defines.h"
#include "GimmickBase.generated.h"

/**
 * Remembers the gimmick config an object found in its world, for an object that reads it often.
 * Only a game world is remembered. The manager of the editor world can get another stage data asset at any time.
 */
struct FGimmickConfigCache
{
	/** @return Config of this type in the world, searched once per game world. Null when the level has none. */
	const UGimmickConfig* Get(const UWorld* World, EGimmickType GimmickType) const;

	/** @return Config of this type in the world, or the defaults of TConfig when the level has none. Never null. */
	template <typename TConfig>
	const TConfig& GetOrDefault(const UWorld* World, const EGimmickType GimmickType) const
	{
		const TConfig* Typed = Cast<TConfig>(Get(World, GimmickType));
		return Typed ? *Typed : *GetDefault<TConfig>();
	}

private:

	/** Config found by Get, or null when the level has none. */
	mutable TWeakObjectPtr<const UGimmickConfig> Config;

	/** Has Get searched already, even when it found nothing. */
	mutable bool bSearched = false;
};

UCLASS()
class TROMBONERUMBLE_API AGimmickBase : public AActor
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	AGimmickBase();
	
	/** Turn the gimmick on. */
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	virtual void Activate();
	
	/** Turn the gimmick off and clear every timer of this actor. */
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	virtual void Deactivate();
	
	/**
	 * Start the gimmick now instead of waiting for its timer.
	 * An event gimmick starts from its warning, and a spawner spawns one object.
	 */
	virtual void ForceTrigger() {}

	/**
	 * Start the timer that moves a gimmick to its next phase.
	 * A duration of 0 or less still moves on, on the next tick. A timer with no time would never fire, and the gimmick would stop there.
	 * Objects that are not gimmicks, such as a spotlight zone or the drunkard state, use it too.
	 *
	 * @param TimerManager Timer manager of the world.
	 * @param Handle Handle of the phase. A running timer on it is replaced.
	 * @param Callback Function that starts the next phase.
	 * @param Duration Seconds the current phase lasts.
	 */
	static void StartPhaseTimer(FTimerManager& TimerManager, FTimerHandle& Handle, const FTimerDelegate& Callback, float Duration);

#if WITH_EDITOR
	/**
	 * Collect objects whose settings differ per kind of spawned object, such as the class default object of each garbage Blueprint.
	 * The gimmick settings panel shows them in its second tab.
	 */
	virtual void GetSettingsObjects(TArray<UObject*>& OutObjects) const {}
#endif

	/** @return Whether the gimmick is on. */
	bool IsActive() const { return bIsActive; }

	/** @return Type that links this gimmick to its config. */
	EGimmickType GetGimmickType() const { return GimmickType; }

protected:

	/** Tell the manager that one event is over, so a sequence can start the next gimmick. Server only. */
	void NotifyEventFinished();

	//~ Begin AActor Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	/**
	 * Get the config of this gimmick.
	 * Call it where you use a value and do not copy the value into a member, so an edit in the editor reaches a running PIE session.
	 *
	 * @return Config of this gimmick, or the class defaults when the level has none.
	 */
	template <typename TConfig>
	const TConfig& GetConfig() const { return ConfigCache.GetOrDefault<TConfig>(GetWorld(), GimmickType); }

	/** Type that links this gimmick to its config. */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
	EGimmickType GimmickType = EGimmickType::None;
	
	/** Is the gimmick on. */
	UPROPERTY(VisibleAnywhere)
	bool bIsActive = false;

private:

	/**
	 * Config of GimmickType in the stage data of the level.
	 * It asks the manager directly instead of waiting for the manager to hand it over,
	 * because a gimmick can need its config before the manager reaches BeginPlay, for example in an OnRep on a client.
	 */
	FGimmickConfigCache ConfigCache;
};
