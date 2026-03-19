#include "UI/UserWidgets/Tutorial/QuestWidget.h"
#include "CommonTextBlock.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/Image.h"

UQuestWidget::UQuestWidget()
{
	CurrentStatus = EQuestStatus::InProgress;
	QuestID = FString();
}

void UQuestWidget::InitQuestWidget(const FQuestUIData& QuestUIData)
{
	QuestID = QuestUIData.QuestID;
	const FText Description = QuestUIData.Description;
	UTexture2D* Icon = QuestUIData.Icon;
	
	if (CT_Description)
	{
		CT_Description->SetText(Description);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CT_Description is not bound in the widget."));
	}
	
	if (Image_Icon)
	{
		// Image_Icon->SetBrushFromTexture(Icon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Icon is not bound in the widget."));
	}
	
	UpdateQuestStatus(EQuestStatus::InProgress);
}

void UQuestWidget::UpdateQuestStatus(EQuestStatus NewStatus)
{
	if (CurrentStatus == NewStatus) return;

	CurrentStatus = NewStatus;
	switch (NewStatus)
	{
		case EQuestStatus::InProgress:
			HandleInProgressStatus();
			break;
		case EQuestStatus::Completed:
			HandleCompletedStatus();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unknown quest status"));
			break;
	}
}

void UQuestWidget::HandleInProgressStatus()
{
	if (Image_Status)
	{
		Image_Status->SetBrushFromTexture(InprogressStatusTexture);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Status is not bound in the widget."));
	}
}

void UQuestWidget::HandleCompletedStatus()
{
	if (Image_Status)
	{
		Image_Status->SetBrushFromTexture(CompleteStatusTexture);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Status is not bound in the widget."));
	}
}