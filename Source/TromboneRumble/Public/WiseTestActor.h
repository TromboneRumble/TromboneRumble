// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "WiseTestActor.generated.h"

UCLASS()
class TROMBONERUMBLE_API AWiseTestActor : public AActor
{
	GENERATED_BODY()

public:
	// 블루프린트에서 호출 가능. Server_PlaySound를 방장, 혹은 클라에서 호출해도 상관없음.
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_PlaySound();

private:
	// 방장의 컴에서 실행됨. 방장 + 모든 클라한테 소리가 재생되도록 호출함.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound();

	// Event가 끝났을 때 새로 함수를 호출시키게 할 수 있다.
	// ex) bgm재생이 끝난 후 게임 종료
	UFUNCTION()
	void OnWwiseCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);

	// 사운드쪽에서는 블루프린트에서 UAkAudioEvent만 연결하면 됨.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> TestSoundEvent;

};
