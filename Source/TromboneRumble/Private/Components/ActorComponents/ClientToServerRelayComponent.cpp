// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/ClientToServerRelayComponent.h"
#include "Interfaces/ServerRPCInterface.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UClientToServerRelayComponent::UClientToServerRelayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UClientToServerRelayComponent::Server_SendRPCRequest_Implementation(AActor* TargetActor)
{
    if (!TargetActor) return;

    ACharacter* InstigatorCharacter = Cast<ACharacter>(GetOwner());
    if (!InstigatorCharacter) return;

    // Target이 서버 RPC를 처리할 수 있는가?
    if (IServerRPCInterface* InterfacePtr = Cast<IServerRPCInterface>(TargetActor))
    {
        InterfacePtr->HandleServerRPC(InstigatorCharacter);
    }
}

