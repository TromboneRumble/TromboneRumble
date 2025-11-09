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
	void Move(const FInputActionValue& Value);
	void Interact();
	void Attack();
	void StartSprint();
	void StopSprint();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	
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

	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedInstrument)
	TObjectPtr<AInstrumentBase> EquippedInstrument = nullptr;

private:
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_SetIsSprinting(const bool bNewIsSprinting);
	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* InteractedActor);
	// ~Server RPCs
	
	// Delegate Callback Handlers
	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);
	UFUNCTION()
	void HandleInteractSuccess(AActor* InteractedActor);
	UFUNCTION()
	void HandleOnRagdoll();
	// ~Delegate Callback Handlers

	// Replication Notifies
	UFUNCTION()
	void OnRep_EquippedInstrument();
	// ~Replication Notifies
	
	void UpdateAttackComponentState();
	void InterpolateMovementSpeed(float DeltaSeconds) const;
	
	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Movement")
	float WalkSpeed = 250.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Movement")
	float SprintSpeed = 600.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Movement")
	float SprintInterpSpeed = 5.0f;
	
	FInteractionContext CurrentInteractionContext;
};
