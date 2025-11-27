// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"

class UNiagaraSystem;
class ARhythmActor;
class UCharacterDataAsset;
class UEquipmentComponent;
class AItemBase;
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
	void TryInteract();
	void Attack();
	void StartSprint();
	void StopSprint();
	void Rhythm(bool bIsPressed);

	EInstrumentType GetCurrentEquippedInstrumentType() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	
	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UEquipmentComponent> EquipmentComponent;
	// ~Components
	
	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmActor> CachedRhythmActor;
	
	FInteractionContext CurrentInteractionContext;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SpotlightSuccessVFX;

private:
	void UpdateMaxWalkSpeed() const;
	float GetCurrentMovementSpeedMultiplier() const;
	
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_SetIsSprinting(const bool bNewIsSprinting);
	UFUNCTION(Server, Reliable)
	void Server_InteractItem(AItemBase* InteractedItem);
	UFUNCTION(Server, Reliable)
	void Server_RequestSpotlightBonus();
	// ~Server RPCs
	
	// Delegate Callback Handlers
	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);
	UFUNCTION()
	void HandleInteractSuccess(AActor* InteractedActor);
	UFUNCTION()
	void HandleOnRagdoll();
	UFUNCTION()
	void HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);
	UFUNCTION()
	void HandleOnNoteDetected(ENoteResult NoteResult);
	// ~Delegate Callback Handlers

	// Multicast RPCs
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySpotlightSuccessEffect();
	// ~Multicast RPCs

	ARhythmActor* GetCachedRhythmActor();
	
	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;
};