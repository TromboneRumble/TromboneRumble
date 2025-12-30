// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/ConfirmationDialogueWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UConfirmationDialogueWidget::ShowDialogue(const FText& Message)
{
	if (CT_Message)
	{
		CT_Message->SetText(Message);
	}
	
	AddToViewport();
}

void UConfirmationDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitButtons();
}

void UConfirmationDialogueWidget::InitButtons()
{
	if (MB_Yes)
	{
		MB_Yes->OnClicked().AddUObject(this, &ThisClass::HandleYesButtonClicked);
	}
	if (MB_No)
	{
		MB_No->OnClicked().AddUObject(this, &ThisClass::HandleNoButtonClicked);
	}
}

void UConfirmationDialogueWidget::HandleYesButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}

void UConfirmationDialogueWidget::HandleNoButtonClicked()
{
	RemoveFromParent();
}
