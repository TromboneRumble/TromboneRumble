// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"

struct FInputActionValue;
class ADefaultPlayerController;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;
class ATrumpet;

UCLASS()
class TROMBONERUMBLE_API ADefaultTromboneCharacter : public ATromboneCharacterBase
{
	GENERATED_BODY()

public:
	ADefaultTromboneCharacter();

	virtual void Jump() override;
	virtual void StopJumping() override;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void Headbutt();
	FORCEINLINE void Sprint() { Server_StartSprint(); }
	FORCEINLINE void StopSprint() { Server_StopSprint(); }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_Interaction(AActor* Interactable);
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();
	UFUNCTION(Server, Reliable)
	void Server_StopSprint();
	// ~Server RPCs

	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;
	// ~Components
private:
	void InterpolateMovementSpeed(float DeltaSeconds) const;

	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);

	UPROPERTY(Replicated)
	uint8 bIsEquipped : 1 = 0;

	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float WalkSpeed = 250.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float SprintSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float SprintInterpSpeed = 10.0f;
};
