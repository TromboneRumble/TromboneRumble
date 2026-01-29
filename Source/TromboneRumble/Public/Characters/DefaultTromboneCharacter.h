// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "DefaultTromboneCharacter.generated.h"


class URageComponent;
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
class URingHitBoxComponent;
class UNiagaraSystem;
class UWidgetComponent;

class ARhythmActor;
class UCharacterDataAsset;
class UWeaponDataAsset;
class UCharacterAttributeSet;
class URhythmScoreAttributeSet;

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
	void ToggleGuideUI();

	EInstrumentType GetCurrentEquippedInstrumentType() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	
	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URageComponent> RageComponent;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributes;

	UPROPERTY()
	TObjectPtr<URhythmScoreAttributeSet> RhythmScoreAttributes;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UClientToServerRelayComponent> ServerRelayComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<URingHitBoxComponent> RingHitBoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|UI")
	TObjectPtr<UWidgetComponent> ComboWidgetComponent;
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
	// ~ Begin Getters / Setters
	FORCEINLINE UClientToServerRelayComponent* GetClientToServerRelayComponent() const { return ServerRelayComponent; }
	FORCEINLINE UAkComponent* GetAkComponent() { return AkSoundComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE TObjectPtr<AWeaponBase> GetCurrentWeapon() const { return AttackComponent ? AttackComponent->GetCurrentWeapon() : nullptr; }
	FORCEINLINE UWidgetComponent* GetComboWidgetComponent() { return ComboWidgetComponent; }
	// ~ End Getters / Setters
};