// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/MainMenuWidget.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"
#include "Components/EditableText.h"
#include "Components/VerticalBox.h"
#include "UI/UserWidgets/ConfirmationDialogueWidget.h"
#include "UI/UserWidgets/MatchMenuWidget.h"
#include "UI/UserWidgets/Settings/AudioOptionPanel.h"
#include "UI/UserWidgets/Settings/VideoOptionPanel.h"

class UOptionPanelBase;

void UMainMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (IsDesignTime()) return;

	InitButtons();
}

void UMainMenuWidget::NativeDestruct()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::InitButtons()
{
	if (CB_Play)
	{
		CB_Play->OnClicked().AddLambda([this] { ChangePanel(Widget_MatchMenu); });
	}
	if (Widget_MatchMenu)
	{
		Widget_MatchMenu->Init([this] { ChangePanel(VB_MainMenu); });
	}
	if (CB_Option)
	{
		CB_Option->OnClicked().AddLambda([this] { ChangePanel(VB_Settings); });
	}
	if (CB_BackFromSettings)
	{
		CB_BackFromSettings->OnClicked().AddLambda([this] { ChangePanel(VB_MainMenu); });
	}
	if (CB_Quit)
	{
		CB_Quit->OnClicked().AddUObject(this, &ThisClass::HandleQuitButtonClicked);
	}
	if (CB_Audio)
	{
		CB_Audio->OnClicked().AddLambda([this] { ChangePanel(Widget_AudioOptions); });
	}
	if (CB_Video)
	{
		CB_Video->OnClicked().AddLambda([this] { ChangePanel(Widget_VideoOptions); });
	}
	if (Widget_AudioOptions)
	{
		Widget_AudioOptions->Init([this] { ChangePanel(VB_Settings); });
	}
	if (Widget_VideoOptions)
	{
		Widget_VideoOptions->Init([this] { ChangePanel(VB_Settings); });
	}
}

void UMainMenuWidget::ChangePanel(UWidget* TargetWidget)
{
	if (CAS_MainMenu)
	{
		CAS_MainMenu->SetActiveWidget(TargetWidget);
	}	
}

void UMainMenuWidget::HandleQuitButtonClicked()
{
	if (!CachedQuitDialog)
	{
		CachedQuitDialog = CreateWidget<UConfirmationDialogueWidget>(GetOwningPlayer(), ConfirmationDialogueWidgetClass);
	}
	// TODO : 메세지 관리
	const FText Message = FText::FromString(TEXT("정말 게임을 나가실건가요?"));
	CachedQuitDialog->ShowDialogue(Message);
}