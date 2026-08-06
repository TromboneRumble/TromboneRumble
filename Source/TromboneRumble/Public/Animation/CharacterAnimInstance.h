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
class ATromboneCharacterBase;
class UCharacterMovementComponent;

UCLASS()
class TROMBONERUMBLE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	void PlayGetUpMontage(bool bIsFacingUp);
	
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionPlayRate = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Instrument")
	EInstrumentType CurrentInstrumentType = EInstrumentType::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Stun")
	bool bIsStunned = false;

private:
	UFUNCTION()
	void OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	
	UPROPERTY(EditDefaultsOnly, Category = "Ragdoll")
	TObjectPtr<UAnimMontage> GetUpFrontMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ragdoll")
	TObjectPtr<UAnimMontage> GetUpBackMontage;

	/** 플레이어 전용 소유자 (악기 타입/이동 배속). NPC에서는 null */
	UPROPERTY(Transient)
	TObjectPtr<ADefaultTromboneCharacter> OwnerCharacter;

	/** 공통 소유자 — 이동/스턴/기상은 베이스 기준으로 처리해 NPC(취객 등)도 이 ABP를 쓸 수 있다 */
	UPROPERTY(Transient)
	TObjectPtr<ATromboneCharacterBase> OwnerBaseCharacter;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> RockFootStepSwitch = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> SnowFootStepSwitch = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> IceFootStepSwitch = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> FabricFootStepSwitch = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> WoodFootStepSwitch = nullptr;

public:
	//~ Begin Setters
	void SetIsAttacking(const bool bNewIsAttacking) { bIsAttacking = bNewIsAttacking; }
	//~ End Setters
};
