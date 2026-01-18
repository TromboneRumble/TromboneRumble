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

			if (APlayerController* PC = GetOwningPlayerController())
			{
				PC->bShowMouseCursor = bShowMouseCursor;

				switch (DefaultInputMode)
				{
				case EInputModeType::UIOnly:
					{
						FInputModeUIOnly InputModeData;
						if (RootLayout) InputModeData.SetWidgetToFocus(RootLayout->TakeWidget());
						InputModeData.SetLockMouseToViewportBehavior(MouseLockMode);
						PC->SetInputMode(InputModeData);
					}
					break;

				case EInputModeType::GameOnly:
					{
						FInputModeGameOnly InputModeData;
						PC->SetInputMode(InputModeData);
					}
					break;

				case EInputModeType::GameAndUI:
					{
						FInputModeGameAndUI InputModeData;
						if (RootLayout) InputModeData.SetWidgetToFocus(RootLayout->TakeWidget());
						InputModeData.SetLockMouseToViewportBehavior(MouseLockMode);
						InputModeData.SetHideCursorDuringCapture(false);
						PC->SetInputMode(InputModeData);
					}
					break;
				}
			}
		}
	}
}
