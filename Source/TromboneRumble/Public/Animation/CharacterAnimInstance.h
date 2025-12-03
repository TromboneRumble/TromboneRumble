// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Utilities/Defines.h"
#include "CharacterAnimInstance.generated.h"

class UAkAudioEvent;
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
	

	
protected:
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

private:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<ADefaultTromboneCharacter> OwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UAkComponent> OwnerAkSoundComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> FootstepAkEvent = nullptr;

public:
	//Getter Setter
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FORCEINLINE void SetIsAttacking(const bool bNewIsAttacking) { bIsAttacking = bNewIsAttacking; };
};
