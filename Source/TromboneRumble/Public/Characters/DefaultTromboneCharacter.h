// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ATrumpet;

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API ADefaultTromboneCharacter : public ATromboneCharacterBase
{
	GENERATED_BODY()

public:

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;



	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> GrabAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> TackleAction;

	float InteractionDistance = 2000.0f;

	UPROPERTY()
	TObjectPtr<class APT_Trumpet> FocusedTrumpet = nullptr;

	UPROPERTY()
	TObjectPtr<class APT_Trumpet> HeldTrumpet = nullptr;

	float TackleAnimationDuration = 1.0f;
};
