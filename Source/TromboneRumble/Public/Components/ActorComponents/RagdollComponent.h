// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/PoseSnapshot.h"
#include "RagdollComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FRagdollUpdatedClient, bool, bInit, bool, bIsFatal, bool, bRecoverStart, bool, bRecoverEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FRagdollUpdatedServer, bool, bInit, bool, bIsFatal, bool, bRecoverStart, bool, bRecoverEnd);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URagdollComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FRagdollUpdatedClient OnRagdollUpdatedClient;
	FRagdollUpdatedServer OnRagdollUpdatedServer;

	UFUNCTION(BlueprintCallable)
	void Client_Ragdoll_Start(bool InIsFatal);

	UFUNCTION(BlueprintCallable)
	void Client_Ragdoll_Recover(bool InDisableNoAnim);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void RPC_Ragdoll_Bake();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CharacterReference();

	UFUNCTION(Client, Reliable)
	void Client_RefreshRagdolls();

	void Client_Ragdoll_Bake();

	

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_Ragdoll_Bake(const TArray<FTransform>& InBonesTransform);

	

	UFUNCTION(Server,Reliable)
	void RPC_Ragdoll_Recover(bool InDisableNoAnim, int32 InPoseId, float InPoseYaw);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Ragdoll_Recover_Start(bool InDisableNoAnim, int32 InPoseId, float InPoseYaw, const FVector& InRecoverVelocity);

	

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Ragdoll_Recover_End();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Rotation();

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_Location();

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_Pose(const TArray<FTransform>& InBonesTransform);

	UFUNCTION(Server,Reliable)
	void RPC_Ragdoll_Start(bool InIsFatal);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Ragdoll_Start(bool InIsFatal, const FVector& InInitVelocity);

	UFUNCTION(Client,Reliable)
	void Client_Ragdoll_Notify(bool InInit, bool InIsFatal, bool InRecoverStart, bool InRecoverEnd);

	UFUNCTION(Server, Reliable)
	void Server_Ragdoll_Notify(bool InInit, bool InIsFatal, bool InRecoverStart, bool InRecoverEnd);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterReference, Category = "References")
	TObjectPtr<ACharacter> characterReference;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<USkeletalMeshComponent> skeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool isWorldOwner = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool isWorldServer = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool isPelvisFound = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdoll = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdollGrounded = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdollRecovering = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdollBaked = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdollFatal = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Booleans")
	bool isRagdollAnimated = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Vectors")
	FVector pelvisOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Vectors")
	FVector capsuleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float ragdollBlend;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float recoverAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float recoverYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float currentAnimDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float ragdollResetTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Float")
	float serverRecoverTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Editable")
	FName pelvisName = TEXT("pelvis");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Editable")
	bool enableAutoRecover = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Editable")
	bool clientSidedPoseCheck = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float autoRecoverTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float autoRecoverVelocity = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float pelvisGroundDistance = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float animationSpringPower = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float capsuleInterp = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float ragdollBlendIn = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float ragdollBlendOut = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float disablePredictionSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Editable")
	float netMaxFPS = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Editable")
	float netMinFPS = 30.f;

	UPROPERTY(EditAnywhere, Category = "Editable")
	TObjectPtr<UAnimMontage> recoverBackwardMontage;

	UPROPERTY(EditAnywhere, Category = "Editable")
	TObjectPtr<UAnimMontage> recoverForwardMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float recoverBackwardApex = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	float recoverForwardApex = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	TArray<TEnumAsByte<EObjectTypeQuery>> traceHitCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Editable")
	TEnumAsByte<EDrawDebugTrace::Type> DebugTraceMode = EDrawDebugTrace::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose")
	FPoseSnapshot serverRagdollPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose")
	FPoseSnapshot serverBakedPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose")
	FPoseSnapshot clientRagdollPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose")
	FPoseSnapshot clientBakedPose;

private:

	void Server_FindPelvisOffset();
	void Server_RefreshRagdolls();

	void TickRagdollBlend();
	void TickUpdateRotation();
	void TickUpdateLocation();
	void TickCapsuleLocation(FVector& OutPelvisLocation, FVector& OutCapsuleLocation, FVector& OutTraceHitLocation,
							 bool& OutTraceHitGround, bool& OutPelvisFullyGrounded, bool& OutPelvisDistanceGrounded, float& OutPelvisDistanceToGround);
	void Server_Auto_Reset();

	FPoseSnapshot SnapshotClientBuild(const TArray<FTransform>& InBonesTransform, const FName& InName = TEXT("baked_pose"));
	FPoseSnapshot SnapshotServerBuild(const FName& InSnapshotName);

	void InitRagdoll(const bool& InIsFatal, const FVector& InInitVelocity);
	float RecoverRagdoll(const bool& InDisableNoAnim, const int32& InPose, const float& InYaw, const FVector& InRecoverVelocity);

	bool CheckReference();
	void GetPelvisRotation(int32& OutPoseId, float& OutPelvisYaw);
	FVector GetCharacterVelocity(bool InUseRagdollVelocity);

	void OnRagdollResetTimerCompleted();
	FTimerHandle RetriggerableDelayHandle;

public:
	FORCEINLINE bool IsRagdoll() const { return isRagdoll; }
};
