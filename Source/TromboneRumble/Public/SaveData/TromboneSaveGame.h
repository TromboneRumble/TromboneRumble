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