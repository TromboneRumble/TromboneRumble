// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GameMapDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "GameMaps Location"))
class TROMBONERUMBLE_API UGameMapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Config : 해당 멤버 변수를 .ini파일에서 읽고 저장하도록 함. 클래스쪽에 (Config=Game/Engine/Editor)로 표시해야함.
	// ForceInlineRow : 한줄로 표현
	UPROPERTY(Config, EditAnywhere, Category = "GameMaps Location", meta = (ForceInlineRow, Categories = "Trombone.Maps"))
	TMap<FGameplayTag, FSoftObjectPath> GamePlayMap;
};
