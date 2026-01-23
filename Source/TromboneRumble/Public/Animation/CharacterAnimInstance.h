// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Utilities/Defines.h"
#include "CharacterAnimInstance.generated.h"

class UAkAudioEvent;
class UAkSwitchValue;
class UAkComponent;
class ADefaultTromboneCharacter;
class UCharacterMovementComponent;

UCLASS()
class TROMBONERUMBLE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	void PlayGetUpMontage(bool bIsFacingUp);
	void SaveRagdollPoseSnapshot();
	
protected:
	UFUNCTION()
	void AnimNotify_FootStep();


	UPROPERTY(BlueprintReadOnly, Category = "Animation")	
	float UpperBodyBlendAlpha = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float BlendInterpSpeed = 15.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "Instrument")
	EInstrumentType CurrentInstrumentType = EInstrumentType::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	bool bIsRagdolling;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	bool bIsRagdollBlending;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	FName RagdollSnapshotName;
	
	UPROPERTY(BlueprintReadOnly, Category = "Stun")
	bool bIsStunned = false;

private:
	UFUNCTION()
	void OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	
	UPROPERTY(EditDefaultsOnly, Category = "Ragdoll")
	TObjectPtr<UAnimMontage> GetUpFrontMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ragdoll")
	TObjectPtr<UAnimMontage> GetUpBackMontage;
	
	UPROPERTY(Transient)
	TObjectPtr<ADefaultTromboneCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UAkComponent> OwnerAkSoundComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> FootstepAkEvent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> NormalFootstepSwitch = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> WaterFootstepSwitch = nullptr;

public:
	//~ Begin Setters
	void SetIsAttacking(const bool bNewIsAttacking) { bIsAttacking = bNewIsAttacking; }
	void SetIsRagdolling(const bool bNewIsRagdolling) { bIsRagdolling = bNewIsRagdolling; }
	//~ End Setters
};
