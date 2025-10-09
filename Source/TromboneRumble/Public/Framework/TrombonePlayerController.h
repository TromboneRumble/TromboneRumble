// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utilities/Defines.h"
#include "TrombonePlayerController.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATrombonePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATrombonePlayerController();
	void ShowInteractionUI(bool bShow) const;

protected:
	virtual void BeginPlay() override;

private:
	EGameState GetGameState() const;
	void InitializeUI();
	void InitializeLobbyUI();
	void InitializeInGameUI();

	UFUNCTION(Server, Reliable)
	void Server_NotifyClientReady();

private:
	TSubclassOf<UUserWidget> InGameUIClass;

	UPROPERTY()
	TObjectPtr<class UPT_UIInGame> InGameUI;
};
