// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunSignature);

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnHitReceived(const FHitData& HitData) override;
	virtual void Tick(float DeltaSeconds) override;

public:
	FOnRagdollSignature OnRagdollDelegate;
	FOnStunSignature OnStunDelegate;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCapsuleComponent> HeadbuttCapsuleComponent;
	
private:
	void InitCharacter();
	void SetupCapsuleComponent();
	void SetupSkeletalMeshComponent() const;
	void SetupMovementComponent() const;

	void OnRagdoll();
	void EndRagdoll();
	void OnStun();
	void EndStun();

	void ApplyStun();
	void UnapplyStun();
	
	void ApplyRagdoll();
	void UnapplyRagdoll();

	// Replication Notifies
	UFUNCTION()
	void OnRep_IsRagdoll();
	UFUNCTION()
	void OnRep_IsStun();
	// ~Replication Notifies

private:
	FTimerHandle OnHitTimerHandle;
	float RagdollDuration = 2.5f;
	float StunDuration = 1.5f;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsStun)
	bool bIsStun = false;
};