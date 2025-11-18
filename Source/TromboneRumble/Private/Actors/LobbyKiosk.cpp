// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/LobbyKiosk.h"
#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Framework/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/Defines.h"

ALobbyKiosk::ALobbyKiosk()
{
	PrimaryActorTick.bCanEverTick = false;
	KioskMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	KioskMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	KioskMeshComp->SetSimulatePhysics(false);
	SetRootComponent(KioskMeshComp);
	
	InteractTrigger = CreateDefaultSubobject<UInteractionTriggerComponent>(TEXT("InteractTrigger"));
}

bool ALobbyKiosk::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return !bIsUsed;
}

void ALobbyKiosk::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		Server_RequestTravel_Implementation();
	}
	else
	{
		Server_RequestTravel();
	}
	bIsUsed = true;
}

void ALobbyKiosk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsUsed);
}

void ALobbyKiosk::Server_RequestTravel_Implementation()
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->RequestServerTravel(EGameState::InGame);
	}
}
