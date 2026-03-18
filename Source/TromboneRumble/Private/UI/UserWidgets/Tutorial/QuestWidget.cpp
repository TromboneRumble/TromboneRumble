#include "UI/UserWidgets/Tutorial/QuestWidget.h"
#include "CommonTextBlock.h"
#include "Actors/Tutorial/TutorialManager.h"

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
		// Image_Icon->SetBrushFromTexture(QuestUIData.Icon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Icon is not bound in the widget."));
	}
}

void UQuestWidget::InitQuestWidget(const FText& Description, UTexture2D* Icon)
{
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
		// Image_Status->SetBrushFromTexture(InProgressIcon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Status is not bound in the widget."));
	}
}

void UQuestWidget::HandleCompletedStatus()
{
	if (CT_Description)
	{
		// TODO : Delete Later
		FText CurrentText = CT_Description->GetText();
		FText StrikethroughText = FText::FromString(FString::Printf(TEXT("<s>%s</s>"), *CurrentText.ToString()));
		CT_Description->SetText(StrikethroughText);
	}
	
	if (Image_Status)
	{
		// Image_Status->SetBrushFromTexture(CompletedIcon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Image_Status is not bound in the widget."));
	}
}