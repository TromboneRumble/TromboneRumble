// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"

class UAttackDataAsset;
class AInstrumentBase;
class UAttackComponent;
struct FInputActionValue;
class ADefaultPlayerController;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;

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
	void Attack();
	FORCEINLINE void Sprint() { Server_StartSprint(); }
	FORCEINLINE void StopSprint() { Server_StopSprint(); }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();
	UFUNCTION(Server, Reliable)
	void Server_StopSprint();
	// ~Server RPCs

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackComponent> AttackComponent;
	// ~Components
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackDataAsset> HeadbuttAttackData;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;

private:
	void InterpolateMovementSpeed(float DeltaSeconds) const;

	// Delegate Callback Handlers
	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);
	UFUNCTION()
	void HandleInteractSuccess(AActor* InteractedActor);
	UFUNCTION()
	void HandleOnRagdoll();
	// ~Delegate Callback Handlers

	UPROPERTY()
	TObjectPtr<AInstrumentBase> EquippedInstrument = nullptr;

	FInteractionContext CurrentInteractionContext;
	
	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float WalkSpeed = 250.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float SprintSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Movement")
	float SprintInterpSpeed = 10.0f;
};
