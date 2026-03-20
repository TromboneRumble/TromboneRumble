#include "UI/UserWidgets/Tutorial/TutorialQuestWidget.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/DynamicEntryBox.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UserWidgets/Tutorial/QuestWidget.h"

UTutorialQuestWidget::UTutorialQuestWidget()
{
}

void UTutorialQuestWidget::HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray)
{
	ClearQuestWidgets();
	
	for (const FQuestUIData& QuestUIData : QuestUIDataArray)
	{
		CreateQuestWidget(QuestUIData);
	}
}

void UTutorialQuestWidget::HandleQuestCompleted(const FString& QuestID)
{
	if (DEB_QuestList)
	{
		for (UUserWidget* EntryWidget : DEB_QuestList->GetAllEntries())
		{
			if (UQuestWidget* QuestWidget = Cast<UQuestWidget>(EntryWidget))
			{
				if (QuestWidget->GetQuestID() == QuestID)
				{
					QuestWidget->UpdateQuestStatus(EQuestStatus::Completed);
					break;
				}
			}
		}
	}
}

void UTutorialQuestWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (TutorialManager)
	{
		TutorialManager->OnQuestSequence.AddUObject(this, &ThisClass::HandleQuestSequence);
		TutorialManager->OnQuestCompleted.AddUObject(this, &ThisClass::HandleQuestCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TutorialManager in the world."));
	}
}


void UTutorialQuestWidget::CreateQuestWidget(const FQuestUIData& QuestUIData)
{
	if (QuestWidgetClass && DEB_QuestList)
	{
		if (UQuestWidget* QuestWidget = DEB_QuestList->CreateEntry<UQuestWidget>(QuestWidgetClass))
		{
			QuestWidget->InitQuestWidget(QuestUIData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create quest widget entry."));
		}
	}
	else
	{
		if (!QuestWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("QuestWidgetClass is not set in the TutorialQuestWidget."));
		}
		if (!DEB_QuestList)
		{
			UE_LOG(LogTemp, Warning, TEXT("DEB_QuestList is not bound in the TutorialQuestWidget."));
		}
	}
}

void UTutorialQuestWidget::ClearQuestWidgets()
{
	if (DEB_QuestList)
	{
		DEB_QuestList->Reset();
	}
}