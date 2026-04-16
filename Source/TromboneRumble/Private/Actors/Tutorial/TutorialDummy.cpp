#include "Actors/Tutorial/TutorialDummy.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/QuestData.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"

void ATutorialDummy::OnHitReceived_Implementation(const FHitData& HitData)
{
	Super::OnHitReceived_Implementation(HitData);
	
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

	if (UTutorialWorldSubsystem* Sub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		Sub->ReportAction(EQuestConditionType::BasicAction, EQuestConditionParamType::Specific, SpecificBasicAction);
	}
}