// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

class UPerformanceWidget;
class ULobbyWidget;

UCLASS()
class TROMBONERUMBLE_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPerformanceWidget> PerformanceWidgetClass;
};
