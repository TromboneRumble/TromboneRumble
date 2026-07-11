#include "Actors/Tutorial/TutorialDummy.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Data/QuestData.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"

ATutorialDummy::ATutorialDummy()
{
	// 더미는 PlayerState가 없어 SkinColor가 Black으로 남는다.
	// 파츠(안테나/몸통)는 컴포넌트 틴트를 꺼 원본 머티리얼을 사용하고,
	// 얼굴/머리(skin/face MID)도 틴트를 꺼 기본 머티리얼 색을 그대로 사용한다.
	CustomizationComp->SetApplyPartsSkinColor(false);
	bApplySkinColorTint = false;
}

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