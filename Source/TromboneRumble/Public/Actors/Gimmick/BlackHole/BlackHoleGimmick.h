// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/EventGimmickBase.h"
#include "BlackHoleGimmick.generated.h"

class ATromboneCharacterBase;
class IConsoleVariable;
class UAkAudioEvent;
class UAkComponent;
class UBlackHoleGimmickConfig;
class UNiagaraComponent;
class AWarningLight;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackHoleBurstSignature, FVector, Center);

/**
 * One place on the ring. Each captured object gets its own, so several of them read as a band
 * rather than as one wire. Server only, and the sign of the rotation is not kept here
 * so that flipping the direction in the settings reaches objects that are already captured.
 */
struct FBlackHoleRingSlot
{
	/** Added to the ring radius, in cm. */
	float RadiusOffset = 0.f;

	/** Height above the center of the hole, in cm. */
	float HeightOffset = 0.f;

	/** How fast this object goes around, in degrees per second. Always positive. */
	float AngularSpeed = 0.f;
};

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
class TROMBONERUMBLE_API ABlackHoleGimmick : public AEventGimmickBase
{
	GENERATED_BODY()

public:

	ABlackHoleGimmick();

	/** State changes on every machine. Props and effects subscribe here instead of being collected. */
	UPROPERTY(BlueprintAssignable, Category = "BlackHole")
	FOnBlackHoleStateChangedSignature OnBlackHoleStateChangedDelegate;

	/**
	 * Fires the instant the collapse throws everything out, with the center of the hole.
	 * Server only, and before the state goes back to Idle, so a subscriber still knows what it had captured.
	 */
	UPROPERTY(BlueprintAssignable, Category = "BlackHole")
	FOnBlackHoleBurstSignature OnBlackHoleBurstDelegate;

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

	/**
	 * Speed the hole wants an object at Location to move at. This is the whole orbit:
	 * the pull curve gives the part toward the center and the orbit curve the part around it.
	 *
	 * @param BaseSpeed       Speed both curve values are a ratio of.
	 * @param bHorizontalOnly Drops the height difference, which walking characters need.
	 * @return Wanted speed, or zero outside the influence radius.
	 */
	FVector ComputeDesiredVelocity(const FVector& Location, float BaseSpeed, bool bHorizontalOnly) const;

	/**
	 * Acceleration that steers a simulating body toward ComputeDesiredVelocity.
	 * Props and ragdolls share it so one set of curves shapes every orbit.
	 *
	 * @param SpeedScale Multiplies the reference speed of this body. 1 is the config value.
	 * @return Acceleration in cm/s2, clamped, or zero outside the influence radius.
	 */
	FVector ComputeSteerAccel(const FVector& Location, const FVector& CurrentVelocity, float SpeedScale) const;

	/**
	 * Speed that holds a captured object on its ring: a spring onto the ring radius and height,
	 * plus a steady turn around the hole. The spring is why a captured object circles instead of
	 * winding into the center, which is the one thing the approach curves cannot do.
	 */
	FVector ComputeRingVelocity(const FVector& Location, const FBlackHoleRingSlot& Slot) const;

	/**
	 * Speed the collapse throws an object at: away from the center, mixed with some up so it arcs.
	 * Props read it as well, which keeps the direction in one place.
	 */
	FVector ComputeBurstVelocity(const FVector& Location) const;

	/**
	 * One place on the ring, jittered per object so several captured things read as a band.
	 * Characters and props both take theirs from here, which keeps the settings in one place.
	 */
	FBlackHoleRingSlot MakeRingSlot() const;

	/** @return Acceleration that moves CurrentVelocity toward DesiredVelocity, with the gain and the ceiling applied. */
	FVector SteerToward(const FVector& DesiredVelocity, const FVector& CurrentVelocity) const;

protected:

	//~ Begin AEventGimmickBase Interface
	virtual const UEventGimmickConfig& GetEventConfig() const override;
	virtual void OnWarningStarted() override;
	virtual void OnEventPhaseStarted(int32 PhaseIndex) override;
	virtual void OnEventEnded() override;
	virtual void OnEventStopped() override;
	//~ End AEventGimmickBase Interface

	/** Called on server and clients when the state changes. The hole effect and sounds already play in C++, so put only extra effects here. */
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

	/** Effect of the hole. Turns on when the hole starts to pull and stops spawning at the burst. Pick the asset, place and size in the Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BlackHole|FX")
	TObjectPtr<UNiagaraComponent> HoleVFXComponent;

	/** Plays the sounds from the center of the hole. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BlackHole|Sound")
	TObjectPtr<UAkComponent> AkComponent;

	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "전조 사운드"))
	TObjectPtr<UAkAudioEvent> WarningSound;

	/** Starts together with the effect. */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "등장 사운드"))
	TObjectPtr<UAkAudioEvent> ActiveSound;

	/** Starts together with the effect and loops until the loop stop event ends it at the burst. */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "반복 사운드"))
	TObjectPtr<UAkAudioEvent> LoopSound;

	/** Plays at the burst and ends the loop sound. */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "반복 정지 이벤트"))
	TObjectPtr<UAkAudioEvent> LoopStopEvent;

	/** The instant the collapse throws everything out and the hole fades away. */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "소멸 사운드"))
	TObjectPtr<UAkAudioEvent> BurstSound;

	/** 예고가 시작될 때 위치 없이 모든 플레이어 화면에서 똑같이 재생되는 경보 사운드. 블랙홀 위치에서 나는 전조 사운드와 따로 재생된다 */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole|Sound", meta = (DisplayName = "경보 사운드"))
	TObjectPtr<UAkAudioEvent> AlarmSound;

	/** Seconds between two capture checks. A player reaches the capture radius at most this late. */
	UPROPERTY(EditDefaultsOnly, Category = "BlackHole", meta = (DisplayName = "포획 판정 주기", ClampMin = "0.02", Units = "s"))
	float CaptureCheckInterval = 0.1f;

private:

	/** Pull everything the hole reaches. Server only, every frame while the hole is on. */
	void TickPull(float DeltaSeconds);

	/** Walking character. Moves the capsule instead of its speed, the same way the blizzard wind does. */
	void PullCharacter(ATromboneCharacterBase* Character, float DeltaSeconds);

	/** Ragdolled character. Rides the ring once captured, still on its way in otherwise. Server only, because clients overwrite that speed every frame. */
	void SteerRagdoll(ATromboneCharacterBase* Character);

	/** Ragdoll and hold every character inside the capture radius. Server only, on a timer. */
	void CheckCaptures();

	/** Ask the character to ragdoll through its combat interface, so invincibility and stun still gate it. */
	void TriggerRagdoll(ATromboneCharacterBase* Character);

	/** Hold one character on the ring: stop its get-up, turn its gravity off, give it a ring slot. Server only. */
	void CaptureCharacter(ATromboneCharacterBase* Character);

	/**
	 * Give every captured character its gravity and its automatic get-up back, and forget it. Server only.
	 *
	 * @param bBurst Throws each one out first. The round ending lets them go without a burst.
	 */
	void ReleaseAllCaptured(bool bBurst);

	/** One arrow showing where the hole wants this object to go. Debug only. LifeTime -1 draws it for this frame alone. */
	void DebugDrawVelocity(const FVector& From, const FVector& Velocity, float LifeTime = -1.f) const;

	/** Phase 0 of the event. The hole opens and grows, and it starts to catch. Server only. */
	void StartGrow();

	/** Phase 1 of the event. The hole stops catching and keeps pulling until it bursts. Server only. */
	void StartCollapse();

	/** Server only. Sets the state and runs the local reaction at once for the listen host. */
	void SetState(EBlackHoleState NewState);

	/** @param PreviousState The engine passes the old value, so a client can tell the burst from the round ending. */
	UFUNCTION()
	void OnRep_State(EBlackHoleState PreviousState);

	/** Runs on every machine after the state changes. Tells the subscribers first, then plays the local reaction. */
	void HandleStateChanged(EBlackHoleState PreviousState);

	/** Turn the hole effect on or off and play the sound of the new state. Not on a dedicated server. */
	void PlayStateEffects(EBlackHoleState PreviousState);

	/** Start every warning light of the level while the hole warns, and fade them out after. Not on a dedicated server. */
	void RefreshWarningLights();

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

	/**
	 * Characters the hole holds until it collapses, each with its place on the ring. Server only.
	 * Leaving the capture radius does not let a character go, because the burst needs the list.
	 */
	TMap<TWeakObjectPtr<ATromboneCharacterBase>, FBlackHoleRingSlot> CapturedCharacters;

	/** How many characters the last server tick pulled. Debug only, stays 0 on clients. */
	int32 PulledCount = 0;

	FTimerHandle CaptureTimerHandle;

	/** Warning lights of the level, found once in BeginPlay. */
	TArray<TWeakObjectPtr<AWarningLight>> WarningLights;

public:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
