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

	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }
	FORCEINLINE bool IsTackling() const { return bIsTackling; }

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultCharacterController> CachedCharacterController;
	// ~Components
private:
	void EndTackleAnimation();

	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);

	UPROPERTY(Replicated)
	uint8 bIsTackling : 1 = 0;

	UPROPERTY(Replicated)
	uint8 bIsEquipped : 1 = 0;

	float TackleAnimationDuration = 1.0f;
};
