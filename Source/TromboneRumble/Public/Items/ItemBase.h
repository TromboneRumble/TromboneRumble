// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "ItemBase.generated.h"

class UAkComponent;
class UInteractionTriggerComponent;
class UCapsuleComponent;
class USphereComponent;
class UAkComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AItemBase();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override PURE_VIRTUAL(AItemBase::Interact_Implementation, );
	// ~ End IInteractable Interfaces

	void SetPhysicsEnabled(bool bEnable) const;
	
	// ~ Begin Components
	UPROPERTY(VisibleAnywhere, Category = "Item|Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Item|Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Components")
	TObjectPtr<UInteractionTriggerComponent> InteractTriggerComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Item|Components|Sound")
	TObjectPtr<UAkComponent> AkSoundComponent = nullptr;
	// ~ End Components

	UFUNCTION()
	virtual void OnRep_CurrentOwner(AActor* OldActor){};

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentOwner)
	TObjectPtr<AActor> CurrentOwner = nullptr;
	
public:
	// ~ Begin Getters & Setters
	FORCEINLINE TObjectPtr<UCapsuleComponent> GetCapsuleComponent() const { return CapsuleComponent; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
	// ~ End Getters & Setters
};