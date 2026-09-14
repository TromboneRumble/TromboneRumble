// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "ItemBase.generated.h"

class UAkComponent;
class UInteractionTriggerComponent;
class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemOwnerChanged, AActor*, NewOwner, AActor*, OldOwner);

UCLASS(Abstract)
class TROMBONERUMBLE_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	
	AItemBase();

public:

	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override PURE_VIRTUAL(AItemBase::Interact_Implementation, );
	// ~ End IInteractable Interfaces
	
	UPROPERTY(BlueprintAssignable, Category = "Item")
	FOnItemOwnerChanged OnOwnerChanged;
	
protected:

	void SetPhysicsEnabled(bool bEnable) const;
	
	UFUNCTION()
	virtual void OnRep_CurrentOwner(AActor* OldActor);
	
protected:
	
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
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentOwner)
	TObjectPtr<AActor> CurrentOwner = nullptr;
	
public:
	
	// ~ Begin Getters & Setters
	FORCEINLINE TObjectPtr<UCapsuleComponent> GetCapsuleComponent() const { return CapsuleComponent; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
	FORCEINLINE AActor* GetCurrentOwner() const { return CurrentOwner; }
	FORCEINLINE bool IsOwned() const { return CurrentOwner != nullptr; }
	// ~ End Getters & Setters
	
public:
	
	// ~ Begin AActor Interfaces
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End AActor Interfaces
	
};