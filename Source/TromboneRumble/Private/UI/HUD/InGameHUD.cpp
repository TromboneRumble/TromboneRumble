// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/InGameHUD.h"
#include "Blueprint/UserWidget.h"
#include "Prototype/InGameWidget.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"

void AInGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (InGameWidgetClass)
	{
		if (UInGameWidget* InGameWidget = CreateWidget<UInGameWidget>(GetWorld(), InGameWidgetClass))
		{
			InGameWidget->AddToViewport();
		}
	}
	
	if (PerformanceWidgetClass)
	{
		if (UPerformanceWidget* PerformanceWidget = CreateWidget<UPerformanceWidget>(GetWorld(), PerformanceWidgetClass))
		{
			PerformanceWidget->AddToViewport();
		}
	}
	
	if (APlayerController* PC = GetOwningPlayerController())
	{
		const FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->bShowMouseCursor = false;
	}
}