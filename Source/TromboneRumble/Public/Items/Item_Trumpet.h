// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Interfaces/Equipable.h"
#include "Item_Trumpet.generated.h"

class UAudioComponent;
class USkeletalMeshComponent;
class UCapsuleComponent;
class UInteractionTriggerComponent;

UCLASS()
class TROMBONERUMBLE_API AItem_Trumpet : public AActor, public IInteractable, public IEquipable
{
	GENERATED_BODY()

public:
	AItem_Trumpet();

	// Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual void Equip_Implementation(AActor* OwnerActor) override;
	virtual void Unequip_Implementation(AActor* OwnerActor) override;
	// ~Interfaces
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
   

private:
    // Components
    UPROPERTY(VisibleAnywhere, Category = "Trumpet")
    TObjectPtr<USkeletalMeshComponent> TrumpetMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Trumpet")
    TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Trumpet|Audio")
    TObjectPtr<UAudioComponent> AudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Interact")
    TObjectPtr<UInteractionTriggerComponent> InteractTrigger = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_Equipped)
    uint8 bIsEquipped : 1 = 0;

    UPROPERTY(Replicated)
    TObjectPtr<AActor> CurrentOwner = nullptr;

    UFUNCTION()
    void OnRep_Equipped();

    // Helpers
    void PlaySound() const;
    void StopSound() const;
    void SetPhysicsEnabled(bool bEnable) const;
    void SetPickupTriggerEnabled_Server(bool bEnable);
    void ReEnableTriggerAfterDrop_Server();

    // Drop Values
    UPROPERTY(EditDefaultsOnly, Category = "Trumpet|Drop")
    float ForwardImpulse = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Trumpet|Drop")
    float UpwardImpulse = 300.f;

    // 장착 소켓
    UPROPERTY(EditDefaultsOnly, Category = "Trumpet|Attach")
    FName AttachSocketName = TEXT("TrumpetSocket");
};
