// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PT_PlayerController.generated.h"

UCLASS()
class TROMBONERUMBLE_API APT_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APT_PlayerController();
	void ShowInteractionUI(bool bShow) const;

protected:
	virtual void BeginPlay() override;

	TSubclassOf<UUserWidget> UIInGameClass;

	UPROPERTY()
	TObjectPtr<class UPT_UIInGame> UIInGame;
};
