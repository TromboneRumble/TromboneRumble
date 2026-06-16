// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TromboneRagdollComponent.generated.h"

class ATromboneCharacterBase;

USTRUCT(BlueprintType)
struct FRagdollNetState
{
	GENERATED_BODY()
	
	/** for reduce bandwidth */
	UPROPERTY()
	FVector_NetQuantize PelvisLocation = FVector::ZeroVector;

	UPROPERTY()
	FQuat PelvisRotation = FQuat::Identity;
	
	UPROPERTY()
	FVector_NetQuantize PelvisVelocity = FVector::ZeroVector;
};

/** URagdollComponent
 * Synchronizes ragdoll simulation in co-op
 * Replicates server's pelvis transform and velocity.
 * Applying velocity interpolation to the pelvis, allowing the rest of the physics body to follow naturally
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UTromboneRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	
	/** Default Constructor */
	UTromboneRagdollComponent();

public:    

	void HandleRagdollChanged(bool bIsRagdoll);

private:
	
	void Server_UpdateRagdollTransform();
	void Client_InterpolateRagdoll(float DeltaTime);

	UFUNCTION()
	void OnRep_ServerRagdollState();
	
protected:
	
	UPROPERTY(EditAnywhere, Category = "RagdollComponent")
	float VelocityInterpSpeed = 15.0f;
	
	/** Network update rate per second */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent")
	float PacketsPerSecond = 30.0f;
	
	/** Squared distance threshold for forcing a hard location snap (cm^2) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent")
	float ForceLocationUpdateDistance = 40000.0f;
	
	/** Tracking intensity factor used to pull pelvis toward target position (P-Control) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent")
	float TrackingIntensity = 10.0f;
	
	/** if true, enables visual debug and screen error logging (== DebugMode)
	 * When the ragdoll state begins or ends, print maximum difference in pelvis between the server and the client during the ragdoll state.
	 */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent")
	bool bEnableDebug = false;
	
private:
	
	UPROPERTY()
	TObjectPtr<ATromboneCharacterBase> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;

	FName PelvisBoneName = TEXT("pelvis");

	UPROPERTY(ReplicatedUsing = OnRep_ServerRagdollState)
	FRagdollNetState ServerRagdollState;

	float TimeSinceLastNetUpdate = 0.0f;
	
	/** Maximum difference for pelvis location synchronization (DebugMode) */
	float PelvisLocationMaxError = 0.0f;

public:
	
	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End UActorComponent Interface
	
};
