// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"

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
	
protected:
	virtual void BeginPlay() override;
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
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttackDataAsset> HeadbuttAttackData;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;
	
	FInteractionContext CurrentInteractionContext;

private:
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_SetIsSprinting(const bool bNewIsSprinting);
	UFUNCTION(Server, Reliable)
	void Server_InteractItem(AItemBase* InteractedItem);
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
	// ~Delegate Callback Handlers
	
	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;
};