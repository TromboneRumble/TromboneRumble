#include "UI/UserWidgets/Tutorial/TutorialWidget.h"
#include "CommonBorder.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/Image.h"
#include "Input/CommonUIInputTypes.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "UI/UserWidgets/Common/FadeWidget.h"
#include "UI/UserWidgets/Tutorial/TutorialDialogueWidget.h"
#include "UI/UserWidgets/Tutorial/TutorialQuestWidget.h"
#include "Utilities/TromboneStatics.h"

UTutorialWidget::UTutorialWidget()
{
	SkipActionHandles.Empty();
	bSupportsActivationFocus = true;
}

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (TutorialManager)
	{
		TutorialManager->OnDialogueSequence.AddUObject(this, &ThisClass::HandleDialogueSequence);
		TutorialManager->OnQuestSequence.AddUObject(this, &ThisClass::HandleQuestSequence);
		TutorialManager->OnTransitionSequence.AddUObject(this, &ThisClass::HandleTransitionSequence);
		TutorialManager->OnShowExtraData.AddUObject(this, &ThisClass::HandleOnExtraData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TutorialManager in the world."));
	}
	
	if (UBaseUIRoot* Root = UTromboneStatics::GetRootLayout(GetOwningPlayer()))
	{
		RootLayout = Root;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find RootLayout for TutorialWidget."));
	}
	
	SetUIVisibility(ESlateVisibility::Collapsed);
}

void UTutorialWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (TutorialManager)
	{
		TutorialManager->OnDialogueSequence.RemoveAll(this);
		TutorialManager->OnQuestSequence.RemoveAll(this);
	}
}

void UTutorialWidget::RegisterInputActions()
{
	if (SkipActionHandles.Num() == 0)
	{
		for (const FDataTableRowHandle& SkipActionRow : SkipActionRowArray)
		{
			if (SkipActionRow.IsNull()) continue;
            
			FBindUIActionArgs BindArgs(SkipActionRow, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleSkipDialogue));
			FUIActionBindingHandle NewHandle = RegisterUIActionBinding(BindArgs);
			if (NewHandle.IsValid())
			{
				SkipActionHandles.Add(NewHandle);
			}
		}
	}
}

void UTutorialWidget::UnregisterInputActions()
{
	for (FUIActionBindingHandle& Handle : SkipActionHandles)
	{
		if (Handle.IsValid())
		{
			Handle.Unregister();
		}
	}
	SkipActionHandles.Empty();
}

void UTutorialWidget::HandleDialogueSequence(const FText& DialogueString)
{
	RegisterInputActions();
	
	SetUIVisibility(ESlateVisibility::Collapsed);
	
	if (WBP_Dialogue)
	{
		WBP_Dialogue->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UTutorialWidget::HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray)
{
	UnregisterInputActions();
	
	SetUIVisibility(ESlateVisibility::Collapsed);
	
	if (WBP_Quest)
	{
		WBP_Quest->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UTutorialWidget::HandleTransitionSequence()
{
	UnregisterInputActions();
	
	if (UFadeWidget* Widget = RootLayout->PushFadeOverlay())
	{
		Widget->OnFadeInComplete.Clear();
		Widget->OnFadeOutComplete.Clear();
		
		Widget->OnFadeInComplete.AddUObject(this, &ThisClass::OnFadeInFinished);
		Widget->OnFadeOutComplete.AddUObject(this, &ThisClass::OnFadeOutFinished);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create FadeWidget for transition sequence."));
		TutorialManager->ProcessTutorial();
	}
}

void UTutorialWidget::HandleSkipDialogue()
{
	if (TutorialManager)
	{
		TutorialManager->ProcessTutorial();
	}
}

void UTutorialWidget::HandleOnExtraData(UTexture2D* Image)
{
	if (Image_ExtraData && Image)
	{
		Image_ExtraData->SetBrushFromTexture(Image, true);
		Image_ExtraData->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	
	if (Border_Dim)
	{
		Border_Dim->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UTutorialWidget::SetUIVisibility(const ESlateVisibility NewVisibility)
{
	if (WBP_Dialogue)
	{
		WBP_Dialogue->SetVisibility(NewVisibility);
	}
	if (WBP_Quest)
	{
		WBP_Quest->SetVisibility(NewVisibility);
	}
	if (Image_ExtraData)
	{
		Image_ExtraData->SetVisibility(NewVisibility);
	}
	if (Border_Dim)
	{
		Border_Dim->SetVisibility(NewVisibility);
	}
}

void UTutorialWidget::OnFadeInFinished()
{
	SetUIVisibility(ESlateVisibility::Collapsed);
}

void UTutorialWidget::OnFadeOutFinished()
{
	if (IsValid(TutorialManager))
	{
		TutorialManager->ProcessTutorial();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialManager is not valid in OnFadeOutFinished."));
		ATutorialManager* RetryManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
		if (IsValid(RetryManager))
		{
			RetryManager->ProcessTutorial();
		}
	}

	if (IsValid(RootLayout))
	{
		RootLayout->PopFadeOverlay();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RootLayout is not valid in OnFadeOutFinished."));
		if (UBaseUIRoot* Root = UTromboneStatics::GetRootLayout(GetOwningPlayer()))
		{
			Root->PopFadeOverlay();
		}
	}
}

TOptional<FUIInputConfig> UTutorialWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::LockAlways, true);
}
