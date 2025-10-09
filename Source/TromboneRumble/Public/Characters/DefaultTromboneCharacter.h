// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"


class ADefaultCharacterController;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;
class ATrumpet;

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API ADefaultTromboneCharacter : public ATromboneCharacterBase
{
	GENERATED_BODY()

public:
	ADefaultTromboneCharacter();

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void Interact();
	void Tackle();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_Interaction(AActor* Interactable);
	UFUNCTION(Server, Reliable)
	void Server_Tackle();
	// ~Server RPCs

	FORCEINLINE bool IsTackling() const { return bIsTackling; }
private:
	void EndTackleAnimation();

	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);

	// Components
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultCharacterController> CachedCharacterController;
	// ~Components

	UPROPERTY(Replicated)
	bool bIsTackling = false;

	UPROPERTY(Replicated)
	bool bHasTempTrumpet = false;
	float TackleAnimationDuration = 1.0f;
};
