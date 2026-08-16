#include "DeveloperSettings/TromboneConfig.h"
#include "Data/UIData.h"
#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"

UTromboneConfig::UTromboneConfig()
	: NoticePopupWidgetClass(UNoticePopup::StaticClass()),
	TwoButtonPopupWidgetClass(UTwoButtonPopup::StaticClass()),
	CharacterSkinColors( {FLinearColor::Red}),
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
	, NoteRingVisualizerClass(FSoftObjectPath(TEXT("/Game/Blueprints/Characters/NoteHitBox/BP_NoteVisualizer.BP_NoteVisualizer_C")))
#endif
{
}

const UTromboneConfig* UTromboneConfig::Get()
{
	return GetDefault<UTromboneConfig>();
}