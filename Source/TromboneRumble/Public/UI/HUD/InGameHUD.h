// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InGameHUD.generated.h"

class UPerformanceWidget;
class UInGameWidget;

UCLASS()
class TROMBONERUMBLE_API AInGameHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInGameWidget> InGameWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPerformanceWidget> PerformanceWidgetClass;
};
