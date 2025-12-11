#include "TromboneGamePlayTags.h"

namespace TromboneGamePlayTags
{
	//태그를 정의
	//TromboneMaps
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_ProtoTypeInGameMap, "Trombone.Maps.ProtoTypeInGameMap");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_MainMenuMap, "Trombone.Maps.MainMenuMap");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_LobbyMap, "Trombone.Maps.LobbyMap");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_Main, "Trombone.Maps.InGame.Main");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_MK, "Trombone.Maps.InGame.MK");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_HW, "Trombone.Maps.InGame.HW");

	// RhythmGame Broadcast Messages
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_OnLayoutChanged, "Trombone.Rhythm.OnLayoutChanged");

	// RhythmGame Broadcast Songs
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_Airplane, "Trombone.Rhythm.Song.Airplane");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapA, "Trombone.Rhythm.Song.MapA");
}