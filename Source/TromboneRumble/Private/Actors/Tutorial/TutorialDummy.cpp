#include "Actors/Tutorial/TutorialDummy.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Kismet/GameplayStatics.h"

void ATutorialDummy::OnHitReceived_Implementation(const FHitData& HitData)
{
	Super::OnHitReceived_Implementation(HitData);
	
	TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	
	FString SpecificBasicAction = FString();
	
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
			if (HasAuthority())
			{
				if (UEquipmentComponent* EquipComp = GetEquipmentComponent())
				{
					EquipComp->Server_UnequipItem_Implementation(EEquipmentSlotType::Weapon);
				}
			}
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