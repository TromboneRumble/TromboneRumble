// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/BaseHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"

void ABaseHUD::PushLoadingOverlay() const
{
	if (RootUI)
	{
		RootUI->PushLoadingOverlay();
	}
}

void ABaseHUD::PushLoadingOverlay(const FString InContent) const
{
	if (RootUI)
	{
		RootUI->PushLoadingOverlay(InContent);
	}
}

void ABaseHUD::PopLoadingOverlay() const
{
	if (RootUI)
	{
		RootUI->PopLoadingOverlay();
	}
}

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
