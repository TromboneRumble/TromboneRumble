// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Utilities/Defines.h"
#include "GimmickBase.generated.h"

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
	const TConfig& GetConfig() const
	{
		const TConfig* Typed = Cast<TConfig>(FindConfig());
		return Typed ? *Typed : *GetDefault<TConfig>();
	}

	/** Type that links this gimmick to its config. */
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
	EGimmickType GimmickType = EGimmickType::None;
	
	/** Is the gimmick on. */
	UPROPERTY(VisibleAnywhere)
	bool bIsActive = false;

private:

	/**
	 * Find the config of GimmickType in the stage data of the level.
	 * The result is cached in a game world only.
	 */
	const UGimmickConfig* FindConfig() const;

	/** Config found by FindConfig, or null when the level has none. */
	mutable TWeakObjectPtr<const UGimmickConfig> CachedConfig;

	/** Has FindConfig searched already, even when it found nothing. */
	mutable bool bConfigSearched = false;
};
