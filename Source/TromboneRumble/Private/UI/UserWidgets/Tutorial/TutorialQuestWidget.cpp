#include "UI/UserWidgets/Tutorial/TutorialQuestWidget.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/DynamicEntryBox.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "UI/UserWidgets/Tutorial/QuestWidget.h"
#include "Utilities/DebugHelper.h"

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

void UTutorialQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnQuestSequenceEvent.AddDynamic(this, &ThisClass::HandleQuestSequence);
		TutorialSub->OnQuestCompletedEvent.AddDynamic(this, &ThisClass::HandleQuestCompleted);
	}
}

void UTutorialQuestWidget::NativeDestruct()
{
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnQuestSequenceEvent.RemoveAll(this);
		TutorialSub->OnQuestCompletedEvent.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UTutorialQuestWidget::CreateQuestWidget(const FQuestUIData& QuestUIData)
{
	if (!QuestWidgetClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("QuestWidgetClass is not set in the TutorialQuestWidget."));
		return;
	}
	if (!DEB_QuestList)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("DEB_QuestList is not bound in the TutorialQuestWidget."));
		return;
	}
	
	if (UQuestWidget* QuestWidget = DEB_QuestList->CreateEntry<UQuestWidget>(QuestWidgetClass))
	{
		QuestWidget->InitQuestWidget(QuestUIData);
	}
	else
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to create quest widget entry"));
	}
}

void UTutorialQuestWidget::ClearQuestWidgets()
{
	if (DEB_QuestList)
	{
		DEB_QuestList->Reset();
	}
}