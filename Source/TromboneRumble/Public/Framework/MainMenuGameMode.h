// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UAkAudioEvent;

UCLASS(Abstract, BlueprintType)
class TROMBONERUMBLE_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "BGM")
	TArray<UAkAudioEvent*> MenuBGMEvents;	
};
