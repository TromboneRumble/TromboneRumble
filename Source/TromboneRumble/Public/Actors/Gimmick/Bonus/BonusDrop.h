// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ServerRPCInterface.h"
#include "Utilities/Defines.h"
#include "BonusDrop.generated.h"

class ACharacter;
class UAkAudioEvent;
class UBonusGimmickConfig;
class USphereComponent;

/**
 * BonusDrop falls from the sky and gives bonus score to the player who touches it.
 * ABonusSpawner drops it, and a subclass decides how it falls and how it looks once it lands.
 *
 * The player who touches it adds the score on its own machine, the same way every other score works, and tells the server through its relay component.
 * The server then removes the drop on every machine.
 * With a lifetime, it disappears that many seconds after it lands.
 *
 * @see ABonusSpawner
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API ABonusDrop : public AActor, public IServerRPCInterface
{
	GENERATED_BODY()

public:

	ABonusDrop();

	/**
	 * Read the score, the lifetime and the settings of a subclass from the config of the spawner. Server only, call it before FinishSpawning.
	 * A subclass with settings of its own calls Super first and then reads its own.
	 */
	virtual void ApplyConfig(const UBonusGimmickConfig& Config);

	/** Set the score for picking it up, for a spawner without a config such as the pressure plate. Server only. */
	void SetBonusScore(const int32 InBonusScore) { BonusScore = InBonusScore; }

	/**
	 * Find the floor straight below a point.
	 *
	 * @return False when there is no floor below.
	 */
	static bool FindGroundBelow(const UWorld* World, const FVector& From, FVector& OutGround);

	//~ Begin IServerRPCInterface Interface
	virtual void HandleServerRPC(ACharacter* InstigatorCharacter) override;
	//~ End IServerRPCInterface Interface

protected:

	/** Move the drop while it falls. Runs on every machine, and the server calls Land when the drop reaches the floor. */
	virtual void TickFalling(float DeltaTime) {}

	/** Runs once on every machine when the drop lands. */
	virtual void OnLanded() {}

	/** Runs on every machine every frame after the drop lands. */
	virtual void TickLanded(float DeltaTime) {}

	/** Fix the landed location, start the lifetime and start the landed state on every machine. Server only. */
	void Land(const FVector& InLandedLocation);

	/** Called on every machine when the drop lands. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bonus", meta = (DisplayName = "On Landed", ScriptName = "OnLanded"))
	void K2_OnLanded();

	/** Called on every machine when a player picks the drop up, just before it disappears. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bonus")
	void OnCollected(ACharacter* Collector);

	/** Touch area of the pickup. Its scale is absolute, so a pop in of the mesh does not shrink it. The subclass attaches it to its root. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> OverlapSphere;

	/** 획득한 플레이어에게만 들리는 효과음 */
	UPROPERTY(EditDefaultsOnly, Category = "Bonus|Audio", meta = (DisplayName = "획득 사운드"))
	TObjectPtr<UAkAudioEvent> CollectSound;

	/** Kind of score the bonus counts as. */
	UPROPERTY(EditDefaultsOnly, Category = "Bonus")
	EScoreType ScoreType = EScoreType::Present;

	UPROPERTY(ReplicatedUsing = OnRep_HasLanded)
	bool bHasLanded = false;

	/** Location the server fixed when the drop landed. */
	UPROPERTY(Replicated)
	FVector LandedLocation = FVector::ZeroVector;

private:

	UFUNCTION()
	void HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_HasLanded();

	/** Give the drop away and remove it on every machine. Server only. */
	void Collect(ACharacter* Collector);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Collected(ACharacter* Collector);

	/** Score for picking it up. Replicated, because the player who touches it adds the score on its own machine. */
	UPROPERTY(Replicated)
	int32 BonusScore = 300;

	/** Seconds it stays after it lands. 0 keeps it. Server only. */
	float Lifetime = 0.f;

	/** Has it been picked up on this machine. Stops a second award before the server answers. */
	bool bCollectedLocally = false;

	/** Has the server given it away. Server only. */
	bool bCollectedOnServer = false;

public:

	//~ Begin AActor Interface
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
};
