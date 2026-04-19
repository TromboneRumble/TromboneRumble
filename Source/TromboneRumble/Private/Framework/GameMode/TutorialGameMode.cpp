#include "Framework/GameMode/TutorialGameMode.h"
#include "Data/QuestData.h"
#include "Items/WeaponBase.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "Utilities/Defines.h"

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
	
	if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		TutorialSub->ReportAction(EQuestConditionType::EquipInstrument, EQuestConditionParamType::Any, FString());
	}
}

void ATutorialGameMode::HandleItemUnequipped(APawn* UnequippedPlayer, AItemBase* UnequippedItem)
{
}
