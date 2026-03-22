#include "Actors/Tutorial/TutorialDummy.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Items/WeaponBase.h"
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

void ATutorialDummy::EquipInstrument(EWeaponType WeaponType)
{
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (const TSubclassOf<AActor>* InstrumentClassPtr = Config->InstrumentClasses.Find(WeaponType))
	{
		if (const TSubclassOf<AActor> InstrumentClass = *InstrumentClassPtr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			
			const FRotator SpawnRotation = FRotator::ZeroRotator;
			const FVector SpawnLocation = GetActorLocation();

			if (AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(InstrumentClass, SpawnLocation, SpawnRotation, SpawnParams))
			{
				if (AWeaponBase* Weapon = Cast<AWeaponBase>(SpawnedInstrument))
				{
					Equip(Weapon);
				}
			}
		}
	}
}