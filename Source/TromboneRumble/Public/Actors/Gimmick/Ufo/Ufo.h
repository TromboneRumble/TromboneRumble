// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/Ufo/UfoPath.h"
#include "GameFramework/Actor.h"
#include "Ufo.generated.h"

class ATromboneCharacterBase;
class UNiagaraComponent;
class UStaticMeshComponent;
class UUfoGimmickConfig;

/**
 * Ufo is the flying object AUfoGimmick spawns.
 * It moves along its path on every machine, and on the server its beam catches the players inside it.
 * The Blueprint adds the UFO mesh, picks the beam collision mesh and the beam effect, and adds the sounds.
 *
 * It warps in from behind its line, spreads its beam down, then flies the line with the beam on.
 * At the end it drops every caught player and folds the beam up, then warps out ahead and destroys itself.
 * Every machine computes the motion from the server time, so only the moment it starts to leave is replicated.
 * The UFO tears off when it starts to leave, and each machine finishes the dash and removes its own copy.
 *
 * @see AUfoGimmick
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API AUfo : public AActor
{
	GENERATED_BODY()

public:

	AUfo();

	/** Set the path. Server only, call it before FinishSpawning. */
	void InitPath(const FUfoPath& InPath) { Path = InPath; }

	/** Forget a player the beam had caught. Server only, the character calls it when it is released. */
	void NotifyBeamLiftEnded(ATromboneCharacterBase* Character);

	/** @return Settings of the UFO gimmick, read through the gimmick that spawned this UFO. The class defaults when it has none. */
	const UUfoGimmickConfig& GetConfig() const;

protected:

	/** Called on every machine when the beam turns on or off. Put the beam effect here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ufo")
	void OnBeamChanged(bool bOn);

	/** Called on every machine when the beam catches a player. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ufo")
	void OnCharacterLifted(ATromboneCharacterBase* Character);

	/** Called on every machine the moment the UFO stops at the start of its line. Put the arrival flash and sound here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ufo")
	void OnWarpIn();

	/** Called on every machine the moment the UFO dashes away. Put the leave flash and sound here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ufo")
	void OnWarpOut();

	/**
	 * Attach the UFO body mesh here. The warp stretches and squashes this pivot, and leaves the beam alone.
	 * The actor faces along its line, so the local X axis of the pivot points where the UFO flies.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ufo")
	TObjectPtr<USceneComponent> BodyPivot;

	/**
	 * Catch volume of the beam, never drawn in game. The server catches the players that overlap its simple collision.
	 * Its pivot must sit at the bottom of the mesh, and the mesh needs a convex simple collision.
	 * The UFO puts it at the end of the beam and stretches it up to the root, so the Blueprint sets only the mesh and the width.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ufo")
	TObjectPtr<UStaticMeshComponent> BeamMesh;

	/** 광선 이펙트. 광선이 펼쳐질 때 켜지고 접히기 시작하면 꺼진다. 이펙트 원점은 광선 아래쪽 끝이어야 한다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ufo")
	TObjectPtr<UNiagaraComponent> BeamVFXComponent;

	/** 광선 이펙트를 세로로 늘리는 배율. 비행 높이가 600일 때 광선 위 끝이 본체 아랫면에 닿도록 맞춘다. 비행 높이가 바뀌면 그 비율만큼 자동으로 따라간다 */
	UPROPERTY(EditDefaultsOnly, Category = "Ufo|VFX", meta = (DisplayName = "광선 VFX Z 스케일", ClampMin = "0.01"))
	float BeamVFXZScale = 1.f;

private:

	/**
	 * Stretch BeamMesh down from the root and hide it when it has no length.
	 *
	 * @param Fraction Part of the beam length to show, from 0 to 1.
	 */
	void FitBeamToLength(float Fraction = 1.f);

	/** Move the UFO, shape its body and its beam, and fire the warp events. Runs on every machine from the server time. */
	void UpdateMotion(float Now);

	/**
	 * Turn the beam effect on or off, once per change. Not on a dedicated server.
	 * The effect stands on the floor and is stretched along Z by BeamVFXZScale, scaled again by the flight height.
	 */
	void SetBeamVFXOn(bool bOn);

	/** Turn the beam on and off, catch players, and start the leave once nobody hangs from the UFO. Server only. */
	void UpdateServer(float Now);

	/** @return Direction along the line, flat on the ground. */
	FVector GetTravelDirection() const;

	/**
	 * Show or hide the beam mesh, then tell the Blueprint. Every machine calls it when the beam turns on or off.
	 * Only the server turns the overlap on, because only the server catches players.
	 */
	void ApplyBeamState();

	/** Drop every player the beam holds. Server only. */
	void ReleaseAllLifted();

	/** Catch every character inside the beam that can be caught now. Server only, runs every tick while the beam is on. */
	void CatchCharactersInBeam();

	/** Server only. */
	void SetBeamOn(bool bOn);

	UFUNCTION()
	void OnRep_BeamOn();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnCharacterLifted(ATromboneCharacterBase* Character);

	UPROPERTY(Replicated)
	FUfoPath Path;

	UPROPERTY(ReplicatedUsing = OnRep_BeamOn)
	bool bBeamOn = false;

	/** Server time the UFO started to leave. Negative until then. */
	UPROPERTY(Replicated)
	float LeaveStartServerTime = -1.f;

	/** Has the beam done its run along the line. Server only, so the beam never turns on twice. */
	bool bBeamFinished = false;

	bool bWarpInFired = false;
	bool bWarpOutFired = false;

	/** Is the beam effect playing on this machine. */
	bool bBeamVFXOn = false;

	/** Have the last particles of the beam effect been removed, at the end of the fold. */
	bool bBeamVFXCut = false;

	/** Players the beam holds right now. Server only. */
	TArray<TWeakObjectPtr<ATromboneCharacterBase>> LiftedCharacters;

public:

	//~ Begin AActor Interface
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void TornOff() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
