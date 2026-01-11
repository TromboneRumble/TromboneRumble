// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterDataAsset.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UPhysicalAnimationComponent;
class UCharacterDataAsset;
class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndStunSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndInvincibleSignature);

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// ~ Begin ICombatReceiver Interfaces
	virtual void OnHitReceived_Implementation(const FHitData& HitData);
	// ~ End ICombatReceiver Interfaces
	
	void ApplySkinColor(const FLinearColor InSkinColor) const;
	void SetPlayerInput(const bool bShouldEnable);

	FOnRagdollSignature OnRagdollDelegate;
	FEndRagdollSignature EndRagdollDelegate;
	FOnStunSignature OnStunDelegate;
	FEndStunSignature EndStunDelegate;
	FOnInvincibleSignature OnInvincibleDelegate;
	FEndInvincibleSignature EndInvincibleDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 SkinMaterialIndex = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 FaceMaterialIndex = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");

private:
	void InitCharacter();
	void SetupCapsuleComponent();
	void SetupSkeletalMeshComponent();
	void SetupCharacterData() const;

	void OnRagdoll();
	void EndRagdoll();
	void OnStun();
	void EndStun();

	void ApplyStun();
	void UnapplyStun();
	
	void ApplyRagdoll();
	void UnapplyRagdoll();
	void DelayedSavePoseSnapshot();
	void InternalUnapplyRagdoll();
	bool IsFacingUp() const;
	float PoseSnapshotInterval = 0.1f;

	void UpdateSkinFromPlayerState();
	
	void ApplyFlagPhysics();

	// Replication Notifies
	UFUNCTION()
	void OnRep_IsRagdoll();
	UFUNCTION()
	void OnRep_IsStun();
	UFUNCTION()
	void OnRep_IsInvincible();
	UFUNCTION()
	void OnRep_SkinColor();
	// ~Replication Notifies

	FTimerHandle OnHitTimerHandle;
	FTimerHandle InvincibilityTimerHandle;

	bool bIsCanProcessInput = true;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsInvincible)
	bool bIsInvincible = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsStun)
	bool bIsStun = false;
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComp;

	FName PelvisBoneName = "pelvis";
	
	// ~ Begin Face Expression Region
	void PlayFaceSequence(ECharacterFaceState TargetState);
	void InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence);
	void ExecuteFaceStep();
	void UpdateFaceExpression(ECharacterFaceType NewType);
	
	int32 CurrentSequenceStep = 0;
	FCharacterFaceAnimationSequence CurrentActiveSequence;
	FTimerHandle FaceSequenceTimerHandle;
	// ~ End Face Expression Region
	
	// TODO : For Debugging
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	UFUNCTION(Server, Reliable)
	void Server_DebugStun();

	UFUNCTION(Server, Reliable)
	void Server_DebugRagdoll();

public:
	//~ Begin Setter
	bool IsStun() const { return bIsStun; }
	bool IsRagdoll() const { return bIsRagdoll; }
	bool IsCanProcessInput() const { return bIsCanProcessInput; }
	//~ End Setter
};