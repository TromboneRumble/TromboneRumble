// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/LobbyHUD.h"
#include "UI/UserWidgets/LobbyWidget.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (LobbyWidgetClass)
	{
		if (ULobbyWidget* LobbyWidget = CreateWidget<ULobbyWidget>(GetWorld(), LobbyWidgetClass))
		{
			LobbyWidget->AddToViewport();
		}
	}
	
	if (PerformanceWidgetClass)
	{
		if (UPerformanceWidget* PerformanceWidget = CreateWidget<UPerformanceWidget>(GetWorld(), PerformanceWidgetClass))
		{
			PerformanceWidget->AddToViewport();
		}
	}
}
