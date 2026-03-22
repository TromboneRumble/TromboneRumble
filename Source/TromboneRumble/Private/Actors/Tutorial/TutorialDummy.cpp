#include "Actors/Tutorial/TutorialDummy.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/EnumHelper.h"

void ATutorialDummy::OnHitReceived_Implementation(const FHitData& HitData)
{
	Super::OnHitReceived_Implementation(HitData);
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	
	FString SpecificBasicAction = FString();
	
	const FString DebugMsg = FString::Printf(TEXT("Hit received from instigator: %s"), *EnumHelper::EnumToString(HitData.HitInstigator));
	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);
	
	switch (HitData.HitInstigator)
	{
		case EHitInstigatorType::Cymbals:
			; // intentional fall through
		
		case EHitInstigatorType::Violin:
			; // intentional fall through
		
		case EHitInstigatorType::Trombone:
			SpecificBasicAction = "HitWithInstrument";
			break;
			
		case EHitInstigatorType::Headbutt:
			SpecificBasicAction = "HitWithHead";
			break;
		
		default:
			break;
	}

	if (TutorialManager)
	{
		TutorialManager->ReportAction(EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, SpecificBasicAction);
	}
}
