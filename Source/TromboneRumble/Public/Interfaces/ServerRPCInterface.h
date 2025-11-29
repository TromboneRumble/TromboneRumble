// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ServerRPCInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UServerRPCInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 클라이언트가 서버 RPC(Server,Reliable)을 호출할 수 있는 Actor는 반드시 그 클라이언트가 소유(Owner)해야 한다.
 *
 * 월드에 배치된 일반 Actor(Spotlight, Item 등)은 어느 클라이언트의 소유가 아니므로
 * Client->Server RPC를 호출할 수 없다.
 *
 * 이 인터페이스는 Owner가 없는 Actor가 서버 로직을 수행하고 싶을때
 * PlayerClass에 ClientToServerRelayComponent가 있는 상태에서 같이 사용된다.
 */
class TROMBONERUMBLE_API IServerRPCInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/// <summary>
	/// 서버에서만 실행되는 RPC 처리 함수
	/// </summary>
	/// <param name="InstigatorCharacter">이 요청을 보낸 플레이어 캐릭터</param>
	virtual void HandleServerRPC(ACharacter* InstigatorCharacter) = 0;
};
