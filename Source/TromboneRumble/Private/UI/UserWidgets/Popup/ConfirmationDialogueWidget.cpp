// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Popup/ConfirmationDialogueWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetSystemLibrary.h"

void UConfirmationDialogueWidget::ShowDialogue(const FText& Message)
{
	SetEnableButtons(true);
	SetVisibility(ESlateVisibility::Visible);
	
	if (CT_Message)
	{
		CT_Message->SetText(Message);
	}
	
	if (!IsInViewport())
	{
		AddToViewport();
	}
	ActivateWidget();
	
	if (FadeIn)
	{
		PlayAnimation(FadeIn);
	}
}

void UConfirmationDialogueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
		
	if (FadeIn)
	{
		PlayAnimation(FadeIn);
	}
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

void UConfirmationDialogueWidget::SetEnableButtons(bool bInIsEnabled)
{
	if (MB_Yes) MB_Yes->SetIsEnabled(bInIsEnabled);
	if (MB_No) MB_No->SetIsEnabled(bInIsEnabled);
}

void UConfirmationDialogueWidget::HandleYesButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}

void UConfirmationDialogueWidget::HandleNoButtonClicked()
{
	SetEnableButtons(false);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	if (FadeIn)
	{
		PlayAnimationReverse(FadeIn);

		const float AnimTime = FadeIn->GetEndTime();
        
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			DeactivateWidget();
		}, AnimTime, false);
	}
	else
	{
		DeactivateWidget();
	}
}