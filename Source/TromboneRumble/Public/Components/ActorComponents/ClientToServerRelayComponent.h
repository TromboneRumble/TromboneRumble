// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClientToServerRelayComponent.generated.h"

/*
 *	클라이언트는 [자신이 소유한 Actor]에만 Server,Reliable RPC를 호출할 수 있다.
 *
 *	Owner가 존재하지 않는 월드 액터(Spotlight, Item 등)가 서버 로직을 수행하고 싶을때
 *	Character가 소유한 이 컴포넌트를 통해 Server RPC를 중계할 수 있다.
 *	
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API UClientToServerRelayComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UClientToServerRelayComponent();

	UFUNCTION(Server, Reliable)
	void Server_SendRPCRequest(AActor* TargetActor);

		
};
