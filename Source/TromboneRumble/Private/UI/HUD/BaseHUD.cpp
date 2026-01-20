// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/BaseHUD.h"
#include "Blueprint/UserWidget.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (RootLayoutClass)
	{
		RootLayout = CreateWidget<UUserWidget>(GetWorld(), RootLayoutClass);
		if (RootLayout)
		{
			RootLayout->AddToViewport();
		}
	}
}
