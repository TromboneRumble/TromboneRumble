#include "UI/UserWidgets/Tutorial/TutorialDialogueWidget.h"
#include "CommonTextBlock.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Kismet/GameplayStatics.h"

UTutorialDialogueWidget::UTutorialDialogueWidget()
{
}

void UTutorialDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (TutorialManager)
	{
		TutorialManager->OnDialogueSequence.AddUObject(this, &ThisClass::SetDialogueText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TutorialManager in the world."));
	}
}

void UTutorialDialogueWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (TutorialManager)
	{
		TutorialManager->OnDialogueSequence.RemoveAll(this);
	}
}

void UTutorialDialogueWidget::SetDialogueText(const FString& DialogueString)
{
	if (CT_Dialogue)
	{
		CT_Dialogue->SetText(FText::FromString(DialogueString));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CT_Dialogue is not bound in the widget."));
	}
}