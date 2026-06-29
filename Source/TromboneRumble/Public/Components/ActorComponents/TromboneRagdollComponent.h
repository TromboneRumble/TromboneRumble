// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TromboneRagdollComponent.generated.h"

class ATromboneCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRagdollSignature);

USTRUCT(BlueprintType)
struct FRagdollNetState
{
	GENERATED_BODY()
	
	/** using FVector_NetQuantize for reduce bandwidth */
	UPROPERTY()
	FVector_NetQuantize PelvisLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize PelvisVelocity = FVector::ZeroVector;
};

/** URagdollComponent
 * Synchronizes ragdoll simulation in co-op
 * Only replicates server's pelvis location and velocity (not rotation).
 * Applying velocity interpolation to the pelvis, allowing the rest of the physics body to follow naturally
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	
	/** Default Constructor */
	UTromboneRagdollComponent();

public:

	/** Start ragdoll. Server only. */
	void StartRagdoll();
	
	/** Stop ragdoll and start get-up animation. Server only. */
	void StopRagdoll();
	
	bool IsRagdoll() const { return bIsRagdoll; }
	
public:

	/** Event when ragdoll is started. */
	FRagdollSignature OnRagdollStarted;
	
	/** Event when ragdoll is ended. (bIsRagdoll = false) */
	FRagdollSignature OnRagdollEnded;
	
	/** Event when ragdoll get-up is completed. */
	FRagdollSignature OnRagdollGetUp;
	
protected:
	
	/** Ragdoll Duration (seconds) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 지속 시간"))
	float RagdollDuration = 2.5f;
	
	/** The interpolation speed while ragdolling, to synchronize with the server's pelvis position */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 중 메쉬의 속도 보간 속도"))
	float VelocityInterpSpeed = 15.0f;
	
	/** Network update rate per second */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "네트워크 업데이트 주기"))
	float PacketsPerSecond = 30.0f;
	
	/** Squared distance threshold for forcing a hard location snap (cm^2) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "골반 위치 강제 동기화 거리"))
	float ForceLocationUpdateDistance = 40000.0f;
	
	/** Tracking intensity factor used to pull pelvis toward target position (P-Control) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "추적 강도"))
	float TrackingIntensity = 10.0f;

	/** The distance traced downward from the pelvis to determine grounding (cm) */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 접지 트레이스 거리"))
	float RagdollGroundTraceDistance = 60.0f;
	
	/** if true, enables visual debug and screen error logging (== DebugMode)
	 * When the ragdoll state begins or ends, print maximum difference in pelvis between the server and the client during the ragdoll state.
	 */
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "디버그 모드"))
	bool bEnableDebug = false;
	
	UPROPERTY(EditAnywhere, Category = "RagdollComponent", meta = (DisplayName = "래그돌 시 하늘로 날리기", EditCondition = "bEnableDebug"))
	bool bEnableImpulseOnRagdollStart = false;

private:

	UFUNCTION()
	void OnRep_IsRagdoll();
	
	void DelayedSavePoseSnapshot();
	
	void UnapplyRagdoll();

	/** @return true if the front of the pelvis is facing toward the sky, otherwise false. */
	bool IsFacingUp() const;
	
	/** @return true if the pelvis is close enough to the ground, otherwise false. */
	bool IsRagdollGrounded() const;

	void Server_UpdateRagdollTransform();
	
	void Client_InterpolateRagdoll(float DeltaTime);

	UFUNCTION()
	void OnRep_ServerRagdollState();
	
private:
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;

	UPROPERTY(ReplicatedUsing = OnRep_ServerRagdollState)
	FRagdollNetState ServerRagdollState;

	float TimeSinceLastNetUpdate = 0.0f;

	/** Time spent on the ground during ragdoll */
	float RagdollGroundedTime = 0.0f;
	
	/** Maximum difference for pelvis location synchronization (DebugMode) */
	float PelvisLocationMaxError = 0.0f;

public:
	
	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End UActorComponent Interface
	
};
