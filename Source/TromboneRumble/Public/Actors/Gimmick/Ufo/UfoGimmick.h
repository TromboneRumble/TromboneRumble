// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Actors/Gimmick/Ufo/UfoPath.h"
#include "Data/Gimmick/UfoGimmickConfig.h"
#include "UfoGimmick.generated.h"

class ATargetPoint;
class AUfo;

UENUM(BlueprintType)
enum class EUfoState : uint8
{
	Idle,

	/** The place of the next UFO is known and shown to the players. */
	Warning,

	/** A UFO is in the level. */
	Active,
};

/** Two points the UFO flies between. Only their ground position counts, the beam length sets the height. */
USTRUCT(BlueprintType)
struct FUfoLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (DisplayName = "시작"))
	TObjectPtr<ATargetPoint> Start;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "끝"))
	TObjectPtr<ATargetPoint> End;

	bool IsValid() const { return Start != nullptr && End != nullptr; }
};

/**
 * UfoGimmick spawns one AUfo each time a sequence of the stage data gives it a turn.
 * Place it in the level and give it the lines the UFO can fly along.
 * Every number comes from UUfoGimmickConfig.
 *
 * Idle, Warning, spawn the UFO, Active until the UFO is gone, then Idle and the sequence moves on.
 *
 * @see AUfo
 * @see UUfoGimmickConfig
 */
UCLASS()
class TROMBONERUMBLE_API AUfoGimmick : public AGimmickBase
{
	GENERATED_BODY()

public:

	AUfoGimmick();

	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface

	/** @return Settings of this gimmick. AUfo reads them through its owner. */
	const UUfoGimmickConfig& GetUfoConfig() const { return GetConfig<UUfoGimmickConfig>(); }

#if WITH_EDITOR
	/** @return Average ground length of the valid lines in cm, or 0 when there is none. The timeline of the settings panel uses it. */
	float GetAverageLineLength() const;
#endif

	/** @return Where the next or the current UFO flies. Only meaningful while the state is not Idle. */
	UFUNCTION(BlueprintPure, Category = "Gimmick")
	FUfoPath GetPlannedPath() const { return PlannedPath; }

protected:

	/** Called on every machine when the state changes. Show the warning at PlannedPath here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gimmick")
	void OnUfoStateChanged(EUfoState NewState);

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
	TSubclassOf<AUfo> UfoClass;

	/**
	 * Lines the UFO can fly along. One is picked at random, and so is its direction.
	 * Only the ground position of each point counts. The UFO flies at the height the beam length sets.
	 */
	UPROPERTY(EditAnywhere, Category = "Gimmick|Config", meta = (DisplayName = "이동 라인 목록"))
	TArray<FUfoLine> Lines;

private:

	/** Pick a line and start the warning, then the event. The sequence of the stage data calls it through ForceTrigger. Server only. */
	void StartWarning();
	void StartActive();

	/** Go back to Idle and tell the manager the turn is over. Server only. */
	void ReturnToIdle();

	UFUNCTION()
	void HandleUfoDestroyed(AActor* DestroyedActor);

	/**
	 * Pick the line the next UFO flies along.
	 *
	 * @param OutPath Start, End and Duration are filled. StartServerTime is not.
	 * @return False when the gimmick has no line.
	 */
	bool PickPath(FUfoPath& OutPath) const;

	/** Server only. Sets the state and runs the local reaction at once for the listen host. */
	void SetState(EUfoState NewState);

	UFUNCTION()
	void OnRep_State();

	UPROPERTY(ReplicatedUsing = OnRep_State)
	EUfoState State = EUfoState::Idle;

	UPROPERTY(Replicated)
	FUfoPath PlannedPath;

	UPROPERTY()
	TObjectPtr<AUfo> ActiveUfo;

	FTimerHandle PhaseTimerHandle;

public:

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	//~ End AActor Interface
};
