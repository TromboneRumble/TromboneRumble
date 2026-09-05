// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/MainMenu/MainMenuWidget.h"
#include "CommonButtonBase.h"
#include "EasyMatchmakingManager.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/AppearanceSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "UI/UserWidgets/Popup/PlayModePopup.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"
#include "Utilities/Defines.h"
#include "Utilities/TromboneStatics.h"

UWidget* UMainMenuWidget::NativeGetDesiredFocusTarget() const
{
	return CB_Play;
}

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (UAppearanceSubsystem* AppearanceSubsystem = GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
	{
		AppearanceSubsystem->ResetColors();
	}
}

void UMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Bound on activate and removed on deactivate. The stack reuses this instance
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().AddDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().AddDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().AddDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}
}

void UMainMenuWidget::NativeOnDeactivated()
{
	if (UEasyMatchmakingManager* MatchmakingManager = UEasyMatchmakingManager::Get(this))
	{
		MatchmakingManager->OnMatchmakingStarted().RemoveDynamic(this, &ThisClass::HandleMatchmakingStarted);
		MatchmakingManager->OnMatchmakingComplete().RemoveDynamic(this, &ThisClass::HandleMatchmakingComplete);
		MatchmakingManager->OnMatchmakingCanceled().RemoveDynamic(this, &ThisClass::HandleMatchmakingCanceled);
	}

	Super::NativeOnDeactivated();
}

void UMainMenuWidget::HandleMatchmakingStarted()
{
	UTromboneStatics::ShowLoadingOverlay(GetOwningPlayer());
}

void UMainMenuWidget::HandleMatchmakingComplete(const FName SessionName, const EEasyMatchmakingCompleteResult Result)
{
	if (Result == EEasyMatchmakingCompleteResult::Failure || Result == EEasyMatchmakingCompleteResult::NoResults)
	{
		// TODO : 로컬라이징
		const FText ToastMessage = FText::FromString(TEXT("Matchmaking failed."));
		UTromboneStatics::ShowToast(GetWorld(), UTromboneStatics::MakeToastRequest(ToastMessage));

		UTromboneStatics::PopOverlay(GetOwningPlayer());
	}
}

void UMainMenuWidget::HandleMatchmakingCanceled()
{
	UTromboneStatics::PopOverlay(GetOwningPlayer());

	if (const UTromboneGameInstance* GI = GetGameInstance<UTromboneGameInstance>())
	{
		const FText ToastMessage = GI->GetCommonUIText("Matchmaking_Cancel");
		UTromboneStatics::ShowToast(GetWorld(), UTromboneStatics::MakeToastRequest(ToastMessage));
	}
}

void UMainMenuWidget::Init()
{
	if (CB_Play)
	{
		CB_Play->OnClicked().RemoveAll(this);
		CB_Play->OnClicked().AddUObject(this, &ThisClass::HandlePlayButtonClicked);
	}
	if (CB_Settings)
	{
		CB_Settings->OnClicked().RemoveAll(this);
		CB_Settings->OnClicked().AddLambda([this]
		{
			UTromboneStatics::ShowPopup<USettingPopup>(GetWorld());
		});
	}
	if (CB_Customize)
	{
		CB_Customize->OnClicked().RemoveAll(this);
		CB_Customize->OnClicked().AddUObject(this, &ThisClass::HandleCustomizeButtonClicked);
	}
	if (CB_Tutorial)
	{
		CB_Tutorial->OnClicked().RemoveAll(this);
		CB_Tutorial->OnClicked().AddUObject(this, &ThisClass::HandleTutorialButtonClicked);
	}
	if (CB_Quit)
	{
		CB_Quit->OnClicked().RemoveAll(this);
		CB_Quit->OnClicked().AddUObject(this, &ThisClass::ShowQuitPopup);
	}
}

void UMainMenuWidget::SetUIEnabled(const bool bEnabled)
{
	if (CB_Play) CB_Play->SetIsEnabled(bEnabled);
	if (CB_Settings) CB_Settings->SetIsEnabled(bEnabled);
	if (CB_Tutorial) CB_Tutorial->SetIsEnabled(bEnabled);
	if (CB_Quit) CB_Quit->SetIsEnabled(bEnabled);
	if (CB_Customize) CB_Customize->SetIsEnabled(bEnabled);
}

void UMainMenuWidget::HandlePlayButtonClicked()
{
	if (TryShowFirstTutorialPopup())
	{
		return;
	}

	UTromboneStatics::ShowPopup<UPlayModePopup>(GetWorld());
}

bool UMainMenuWidget::TryShowFirstTutorialPopup() const
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		if (Subsystem->ShouldShowTutorialPopup())
		{
			ShowTutorialPopup();
			Subsystem->MarkTutorialAsCompleted();
			return true;
		}
	}
	return false;
}

void UMainMenuWidget::ShowTutorialPopup() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupTitle"));
		Params.Content = GI->GetTutorialUIText(TEXT("StringKey_TutorialFirstPlayerShowPopupDescription"));
		Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		Params.RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));

		Params.LeftCallback = [this]
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Tutorial);
		};

		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			Popup->Init(Params);
		}
	}
}

void UMainMenuWidget::HandleCustomizeButtonClicked()
{
	UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Customize);
}

void UMainMenuWidget::HandleTutorialButtonClicked()
{
	if (USaveManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		Subsystem->MarkTutorialAsCompleted();
	}
	
	UTromboneStatics::OpenLevel(GetWorld(), ELevelType::Tutorial);
}

void UMainMenuWidget::ShowQuitPopup() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = FText::GetEmpty();
		Params.Content = GI->GetCommonUIText(TEXT("Confirmation_QuitGame"));
		Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		Params.RightButtonText = GI->GetCommonUIText(TEXT("Common_No"));
    
		Params.LeftCallback = [this]()
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
			}
		};
		
		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			Popup->Init(Params);
		}
	}
}
