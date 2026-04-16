#include "UI/UserWidgets/Tutorial/TutorialDialogueWidget.h"
#include "CommonTextBlock.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"

void UTutorialDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnDialogueSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialDialogueSequence);
	}
}

void UTutorialDialogueWidget::NativeDestruct()
{
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->OnDialogueSequenceEvent.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UTutorialDialogueWidget::OnTutorialDialogueSequence(const FText& DialogueString)
{
	if (CT_Dialogue)
	{
		CT_Dialogue->SetText(DialogueString);
	}

	if (BounceAnim && Image_Speaker)
	{
		PlayAnimation(BounceAnim);
	}
}