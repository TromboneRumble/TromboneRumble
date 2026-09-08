#include "DeveloperSettings/TromboneConfig.h"
#include "Data/UIData.h"
#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"

UTromboneConfig::UTromboneConfig()
	: CharacterSkinColors( {FLinearColor::Red}),
	RoomCodeLength(5), 
	DefaultInGameMap(TromboneGamePlayTags::Trombone_Maps_InGame_OrchestraStage), 
	LobbyGameStartDelaySeconds(5),
	LobbyRagdollGetUpDelaySeconds(5),
	ToastSystemPolicy(EToastSystemPolicy::Queue),
	IntervalAfterQuestCompletion(1.5f), 
	TutorialStartDelay(1.0f),
	PerformanceWidgetUpdateInterval(0.5f)
#if WITH_EDITORONLY_DATA
	, NoteRingBaseMaterial(FSoftObjectPath(TEXT("/Game/Blueprints/Characters/NoteHitBox/M_NoteVisualizer.M_NoteVisualizer")))
	, NoteRingAnchorCharacterClass(FSoftObjectPath(TEXT("/Game/Blueprints/Characters/BP_DefaultCharacter.BP_DefaultCharacter_C")))
#endif
{
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}

TSoftClassPtr<UCommonActivatableWidget> UTromboneConfig::GetPopupClass(const UClass* PopupType) const
{
	// Walk toward the root so a subclass falls back to its parent's entry
	for (const UClass* Class = PopupType; Class; Class = Class->GetSuperClass())
	{
		for (const FPopupClassEntry& Entry : PopupClasses)
		{
			if (Entry.PopupType == Class)
			{
				return Entry.WidgetClass;
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("No popup entry for %s in Trombone Config."), *GetNameSafe(PopupType));
	return nullptr;
}