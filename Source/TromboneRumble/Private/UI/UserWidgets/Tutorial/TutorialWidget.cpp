#include "UI/UserWidgets/Tutorial/TutorialWidget.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Input/CommonUIInputTypes.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/Tutorial/TutorialDialogueWidget.h"
#include "UI/UserWidgets/Tutorial/TutorialQuestWidget.h"

UTutorialWidget::UTutorialWidget()
{
	SkipActionHandles.Empty();
}

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (TutorialManager)
	{
		TutorialManager->OnDialogueSequence.AddUObject(this, &ThisClass::HandleDialogueSequence);
		TutorialManager->OnQuestSequence.AddUObject(this, &ThisClass::HandleQuestSequence);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TutorialManager in the world."));
	}
	
	if (WBP_Dialogue)
	{
		WBP_Dialogue->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (WBP_Quest)
	{
		WBP_Quest->SetVisibility(ESlateVisibility::Collapsed);
	}
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
	
	if (WBP_Dialogue)
	{
		WBP_Dialogue->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	
	if (WBP_Quest)
	{
		WBP_Quest->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTutorialWidget::HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray)
{
	UnregisterInputActions();
	
	if (WBP_Quest)
	{
		WBP_Quest->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (WBP_Dialogue)
	{
		WBP_Dialogue->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTutorialWidget::HandleSkipDialogue()
{
	if (!TutorialManager) return;
	
	TutorialManager->ProcessTutorial();
}
