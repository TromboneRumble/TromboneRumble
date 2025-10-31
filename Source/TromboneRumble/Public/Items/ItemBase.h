// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "ItemBase.generated.h"

class UInteractionTriggerComponent;
class UCapsuleComponent;
class USphereComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AItemBase();
	
	FORCEINLINE TObjectPtr<UCapsuleComponent> GetCapsuleComponent() const { return CapsuleComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override; 
	virtual void Interact_Implementation(AActor* InstigatorActor) override PURE_VIRTUAL(AItemBase::Interact_Implementation, );
	// ~Interfaces

	// Overlap Events
	UFUNCTION()
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// ~Overlap Events

protected:
	void SetPhysicsEnabled(bool bEnable) const;
	
	// Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ItemMeshComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInteractionTriggerComponent> InteractTriggerComponent = nullptr;
	// ~Components

	UPROPERTY(VisibleAnywhere, Replicated)
	TObjectPtr<AActor> CurrentOwner = nullptr;
};
