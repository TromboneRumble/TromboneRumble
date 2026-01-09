// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TromboneSaveGame.generated.h"

USTRUCT()
struct FAudioSettingData 
{
	GENERATED_BODY()
	
	UPROPERTY() float MasterVolume = 0.5f;
	UPROPERTY() float MusicVolume = 0.5f;
	UPROPERTY() float SFXVolume = 0.5f;
};

USTRUCT()
struct FGameplaySettingData 
{
	GENERATED_BODY()
	
	UPROPERTY() float TestProperty = 0.0f;
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
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::Windowed;

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
};