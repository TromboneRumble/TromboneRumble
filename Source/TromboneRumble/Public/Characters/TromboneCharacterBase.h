// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollSignature);

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnHitReceived(const FHitData& HitData) override;

public:
	FOnRagdollSignature OnRagdollDelegate;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCapsuleComponent> HeadbuttCapsuleComponent;
	
private:
	void InitCharacter();
	void SetupCapsuleComponent();
	void SetupSkeletalMeshComponent() const;
	void SetupMovementComponent() const;

	void OnRagdoll();
	
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