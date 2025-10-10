// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "LobbyKiosk.generated.h"

class UCapsuleComponent;
class UInteractionTriggerComponent;

UCLASS()
class TROMBONERUMBLE_API ALobbyKiosk : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	ALobbyKiosk();
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION(Server, Reliable)
	void Server_RequestTravel();

private:
	UPROPERTY()
	UStaticMeshComponent* KioskMeshComp;

	UPROPERTY(EditAnywhere)
	UStaticMesh* KioskMesh;

	UPROPERTY()
	TObjectPtr<UInteractionTriggerComponent> InteractTrigger = nullptr;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

	bool bIsUsed = false;
};