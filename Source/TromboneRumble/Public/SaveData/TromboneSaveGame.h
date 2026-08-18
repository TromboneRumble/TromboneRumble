// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Utilities/Defines.h"
#include "TromboneSaveGame.generated.h"

USTRUCT()
struct FPlayerData
{
	GENERATED_BODY()
	
	UPROPERTY() bool bIsFirstTimePlayer = true;
};

USTRUCT()
struct FAudioSettingData
{
	GENERATED_BODY()

	UPROPERTY() float MasterVolume = 0.5f;
	UPROPERTY() float BGMVolume = 0.5f;
	UPROPERTY() float MusicVolume = 0.5f;
	UPROPERTY() float SFXVolume = 0.5f;
	UPROPERTY() float UIVolume = 0.5f;

	UPROPERTY() float VoiceSendVolume = 1.0f;        // 내 목소리 전송 볼륨 (0.0 ~ 2.0)
	UPROPERTY() int32 MicrophoneDeviceIndex = 0;     // 선택된 마이크 장치 인덱스
	UPROPERTY() FString MicrophoneDeviceName = TEXT(""); // 장치 연결 변경 시 이름 기준 재매칭용
	UPROPERTY() bool bNoiseSuppression = true;       // 보이스챗 잡음 제거

	// 양수 = 이 PC에서 소리가 그만큼 늦게 들림 → BGM을 그만큼 일찍 시작
	UPROPERTY() int32 RhythmAudioOffsetMs = 0;
};

USTRUCT()
struct FGameplaySettingData 
{
	GENERATED_BODY()
	
	UPROPERTY() 
	bool bShouldShowUsernameInGame = true;
	
	UPROPERTY()
	EVoipMode VOIPSetting = EVoipMode::PushToTalk;
	
};

USTRUCT(BlueprintType)
struct FGraphicsSettingData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OverallQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewDistance = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AntiAliasing = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PostProcess = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Shadow = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GlobalIllumination = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Reflections = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Texture = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Effects = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Resolution = FIntPoint(1920, 1080);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVSync = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::Fullscreen;

	FGraphicsSettingData() {}
};

UCLASS()
class TROMBONERUMBLE_API UTromboneSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY() 
	FAudioSettingData Audio;
	
	UPROPERTY() 
	FGameplaySettingData Gameplay;
	
	UPROPERTY() 
	FPlayerData PlayerData;
};