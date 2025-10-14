// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UInputComponent;

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnHitReceived(const FHitData& HitData) override;
	
private:
	void InitCharacter() const;
	void SetupCapsuleComponent() const;
	void SetupSkeletalMeshComponent() const;
	void SetupMovementComponent() const;
	
	void ApplyRagdoll();
	void UnapplyRagdoll();

	UFUNCTION()
	void OnRep_IsRagdoll();

private:
	FTimerHandle RagdollTimerHandle;
	float RagdollDuration = 3.0f;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;
};