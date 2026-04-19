#include "UI/UserWidgets/Tutorial/TutorialWidget.h"
#include "CommonBorder.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/Image.h"
#include "Input/CommonUIInputTypes.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "UI/UserWidgets/Common/FadeWidget.h"
#include "UI/UserWidgets/Tutorial/TutorialDialogueWidget.h"
#include "UI/UserWidgets/Tutorial/TutorialQuestWidget.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/TromboneStatics.h"

UTutorialWidget::UTutorialWidget()
{
	SkipActionHandles.Empty();
	bSupportsActivationFocus = true;
}

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnDialogueSequenceEvent.AddDynamic(this, &ThisClass::HandleDialogueSequence);
		TutorialSub->OnQuestSequenceEvent.AddDynamic(this, &ThisClass::HandleQuestSequence);
		TutorialSub->OnTransitionSequenceEvent.AddDynamic(this, &ThisClass::HandleTransitionSequence);
		TutorialSub->OnShowExtraDataEvent.AddDynamic(this, &ThisClass::HandleOnExtraData);
	}
	
	if (UBaseUIRoot* Root = UTromboneStatics::GetRootLayout(GetOwningPlayer()))
	{
		RootLayout = Root;
	}
	else
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to find RootLayout for TutorialWidget."));
	}
	
	SetUIVisibility(ESlateVisibility::Collapsed);
}

void UTutorialWidget::NativeDestruct()
{
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnDialogueSequenceEvent.RemoveAll(this);
		TutorialSub->OnQuestSequenceEvent.RemoveAll(this);
		TutorialSub->OnTransitionSequenceEvent.RemoveAll(this);
		TutorialSub->OnShowExtraDataEvent.RemoveAll(this);
	}
	
	UnregisterInputActions();
	
	Super::NativeDestruct();
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
		if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
		{
			TutorialSub->ProcessTutorial();
		}
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to push FadeWidget for transition sequence."));
	}
}

void UTutorialWidget::HandleSkipDialogue()
{
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->ProcessTutorial();
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
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->ProcessTutorial();
	}

	if (IsValid(RootLayout))
	{
		RootLayout->PopFadeOverlay();
	}
	else
	{
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
