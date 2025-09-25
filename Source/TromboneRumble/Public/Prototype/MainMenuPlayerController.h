// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

UCLASS()
class TROMBONERUMBLE_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMainMenuPlayerController();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

protected:
	virtual void BeginPlay() override;

	TSubclassOf<UUserWidget> UIMainClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> UIMain;
};
