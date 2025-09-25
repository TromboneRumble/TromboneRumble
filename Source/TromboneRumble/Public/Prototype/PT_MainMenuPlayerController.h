// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PT_MainMenuPlayerController.generated.h"

UCLASS()
class TROMBONERUMBLE_API APT_MainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APT_MainMenuPlayerController();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

protected:
	virtual void BeginPlay() override;

	TSubclassOf<UUserWidget> UIMainClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> UIMain;
};
