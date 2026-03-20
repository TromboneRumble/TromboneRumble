#include "Framework/GameMode/TutorialGameMode.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Data/QuestData.h"
#include "Items/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/Defines.h"

ATutorialGameMode::ATutorialGameMode()
{
	bUseSeamlessTravel = true;
}

void ATutorialGameMode::HandleItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	if (!EquippedPlayer || !EquippedItem) return;
	
	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(EquippedItem))
	{
		if (Weapon->GetWeaponType() == EWeaponType::Headbutt)
		{
			return;
		}
	}
	
	ATutorialManager* TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (TutorialManager)
	{
		TutorialManager->ReportAction(EQuestConditionType::EquipInstrument, EQuestConditionParamType::Any, FString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TutorialManager in the world."));
	}
}

void ATutorialGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}
