// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "BlackHoleGimmick.generated.h"

class IConsoleVariable;
class UBlackHoleGimmickConfig;
class USphereComponent;

/** Level materials read these numbers from the parameter collection. Do not change the values. */
UENUM(BlueprintType)
enum class EBlackHoleState : uint8
{
	Idle = 0,
	Warning = 1,
	Active = 2,
	Collapse = 3,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackHoleStateChangedSignature, EBlackHoleState, NewState);

/**
 * ABlackHoleGimmick pulls players and props around one fixed point and throws the captured ones out when it collapses.
 * Place it in the level by hand. Its own transform is the center of the hole.
 *
 * Keep in mind that the gimmick holds no list of props.
 * It publishes the state and the pull field, and UBlackHolePullableComponent subscribes on its own
 * and moves the body it sits on. A prop without that component is never touched.
 *
 * Both radii grow from ActiveStartServerTime, so every machine works them out on its own
 * and no radius is replicated.
 *
 * @see UBlackHoleGimmickConfig
 */
UCLASS()
class TROMBONERUMBLE_API ABlackHoleGimmick : public AGimmickBase
{
	GENERATED_BODY()

public:

	ABlackHoleGimmick();

	//~ Begin AGimmickBase Interface
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void ForceTrigger() override;
	//~ End AGimmickBase Interface

	/** State changes on every machine. Props and effects subscribe here instead of being collected. */
	UPROPERTY(BlueprintAssignable, Category = "BlackHole")
	FOnBlackHoleStateChangedSignature OnBlackHoleStateChangedDelegate;

	EBlackHoleState GetState() const { return State; }

	/** @return Is the hole pulling right now, which is true while it grows and while it collapses. */
	UFUNCTION(BlueprintPure, Category = "BlackHole")
	bool IsPulling() const { return State == EBlackHoleState::Active || State == EBlackHoleState::Collapse; }

	/** @return How far the hole has grown. 0 while idle, 0 to 1 while growing, 1 while collapsing. */
	UFUNCTION(BlueprintPure, Category = "BlackHole")
	float GetGrowthAlpha() const;

	/** @return Radius the pull reaches right now. */
	UFUNCTION(BlueprintPure, Category = "BlackHole")
	float GetInfluenceRadius() const;

	/** @return Radius that ragdolls a player and captures a prop right now. */
	UFUNCTION(BlueprintPure, Category = "BlackHole")
	float GetInnerRadius() const;

	bool IsInsideInfluence(const FVector& Location) const;
	bool IsInsideInner(const FVector& Location) const;

protected:

	/** Called on server and clients when the state changes. Put sound and particles here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BlackHole")
	void OnBlackHoleStateChanged(EBlackHoleState NewState);

	// The settings a designer tunes are in UBlackHoleGimmickConfig. Only references stay here

	/**
	 * Shows the capture radius in the editor and gives the effects something to attach to.
	 * Collision stays off on purpose, because a ragdoll has no capsule collision and would
	 * never raise an overlap. The capture check measures distances instead.
	 */
	UPROPERTY(VisibleAnywhere, Category = "BlackHole")
	TObjectPtr<USphereComponent> InnerSphere;

private:

	/** Start the timer of the next warning. Server only. */
	void ScheduleNext(float Delay);
	void StartWarning();
	void StartActive();
	void StartCollapse();
	void EndCollapse();

	/** Server only. Sets the state and runs the local reaction at once for the listen host. */
	void SetState(EBlackHoleState NewState);

	UFUNCTION()
	void OnRep_State();

	/** Runs on every machine after the state changes. Tells the subscribers first, then plays the local reaction. */
	void HandleStateChanged();

	/** Tick runs while the hole is on, and while Trombone.BlackHole.Debug is on. */
	void UpdateTickEnabled();
	void HandleDebugCVarChanged(IConsoleVariable* Variable);

	/** Two circles and one line on screen: state, time left, radii. */
	void DebugDraw() const;

	UPROPERTY(ReplicatedUsing = OnRep_State)
	EBlackHoleState State = EBlackHoleState::Idle;

	/** Server time the growth started. Every machine derives both radii from it. */
	UPROPERTY(Replicated)
	float ActiveStartServerTime = 0.f;

	FTimerHandle ScheduleTimerHandle;
	FTimerHandle PhaseTimerHandle;

public:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
