// Fill out your copyright notice in the Description page of Project Settings.


#include "WiseTestActor.h"
#include "Utilities/DebugHelper.h"

// 방장이든 클라든 상관없이 방장보고 사운드를 재생해달라고 호출함.
void AWiseTestActor::Server_PlaySound_Implementation()
{
	//방장은 자신 포함 모든 클라한테 사운드 재생하라고 호출
	Multicast_PlaySound();
}

void AWiseTestActor::Multicast_PlaySound_Implementation()
{
    //Wwise Event를 제대로 연결했다면 재생
    if (TestSoundEvent)
    {
		//사운드 재생이 끝났을 때 OnWwiseCallback 함수를 호출하도록 설정
        FOnAkPostEventCallback OnCallback;
        OnCallback.BindDynamic(this, &ThisClass::OnWwiseCallback);

        //사운드 재생
        UAkGameplayStatics::PostEvent(TestSoundEvent, this, AK_EndOfEvent, OnCallback);
    }
}

void AWiseTestActor::OnWwiseCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
    Debug::Print(TEXT("사운드 재생 완료"));
}
