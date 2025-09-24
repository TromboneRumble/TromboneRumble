// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InGamePlayerController.generated.h"

UCLASS()
class TROMBONERUMBLE_API AInGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AInGamePlayerController();
	void ShowInteractionUI(bool bShow) const;

protected:
	virtual void BeginPlay() override;

protected:
	TSubclassOf<UUserWidget> UIInGameClass;

	UPROPERTY()
	TObjectPtr<class UUIInGame> UIInGame;
};
