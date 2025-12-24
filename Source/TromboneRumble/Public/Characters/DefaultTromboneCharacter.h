// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Characters/TromboneCharacterBase.h"
#include "DefaultTromboneCharacter.generated.h"


class AWeaponBase;
struct FInputActionValue;
class ADefaultPlayerController;

class UEquipmentComponent;
class UAkComponent;
class UClientToServerRelayComponent;
class UAttackComponent;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;
class UAbilitySystemComponent;
class UNiagaraSystem;

class ARhythmActor;
class UCharacterDataAsset;
class UWeaponDataAsset;
class UCharacterAttributeSet;

class AItemBase;

UCLASS()
class TROMBONERUMBLE_API ADefaultTromboneCharacter : public ATromboneCharacterBase, public IAbilitySystemInterface
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

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAkComponent> AkSoundComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributes;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UClientToServerRelayComponent> ServerRelayComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> JudgementRingComponent;
	// ~Components
	
	UPROPERTY(EditDefaultsOnly, Category = "DefaultWeapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> DefaultWeaponInstance = nullptr;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmActor> CachedRhythmActor;

private:
	void UpdateMaxWalkSpeed();
	
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

	ARhythmActor* GetCachedRhythmActor();
	void SpawnAndEquipDefaultWeapon();
	void SpawnAndEquipPreviouslyEquippedWeapon();
	
	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;

public:
	//getter setter
	FORCEINLINE UClientToServerRelayComponent* GetClientToServerRelayComponent() const { return ServerRelayComponent; }
	FORCEINLINE UAkComponent* GetAkComponent() { return AkSoundComponent; }
	FORCEINLINE UStaticMeshComponent* GetJudgementRingComponent() { return JudgementRingComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
};