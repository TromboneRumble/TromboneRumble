// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/BaseHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (RootUIClass)
	{
		RootUI = CreateWidget<UBaseUIRoot>(GetWorld(), RootUIClass);
		if (RootUI)
		{
			RootUI->AddToViewport();
		}
	}
}
