#include "TromboneGamePlayTags.h"

namespace TromboneGamePlayTags
{

	// 게임 흐름에 사용되는 맵들 (인게임 제외 OutGame들)
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_OutGame_MainMenu,			"Trombone.Maps.OutGame.MainMenu");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_OutGame_MatchMenu,			"Trombone.Maps.OutGame.MatchMenu");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_OutGame_Tutorial,			"Trombone.Maps.OutGame.Tutorial");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_OutGame_Customize, 		"Trombone.Maps.OutGame.Customize");
	
	// 결과 맵
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Result_OrchestraStage,		"Trombone.Maps.Result.OrchestraStage");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Result_SnowField,			"Trombone.Maps.Result.SnowField");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Result_JazzBar,			"Trombone.Maps.Result.JazzBar");

	// 로비 맵
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Lobby_OrchestraStage,		"Trombone.Maps.Lobby.OrchestraStage");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Lobby_SnowField,			"Trombone.Maps.Lobby.SnowField");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Lobby_JazzBar,				"Trombone.Maps.Lobby.JazzBar");
	
	// 인게임 맵
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_OrchestraStage,		"Trombone.Maps.InGame.OrchestraStage");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_SnowField,			"Trombone.Maps.InGame.SnowField");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_InGame_JazzBar,			"Trombone.Maps.InGame.JazzBar");

	// 테스트 맵
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Test_Proto,				"Trombone.Maps.Test.Proto");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Test_MK,					"Trombone.Maps.Test.MK");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Test_HW,					"Trombone.Maps.Test.HW");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Maps_Test_MJ,					"Trombone.Maps.Test.MJ");

	// RhythmGame Broadcast Messages
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_OnLayoutChanged, "Trombone.Rhythm.OnLayoutChanged");

	// RhythmGame Broadcast Songs
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_Airplane, "Trombone.Rhythm.Song.Airplane");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapA, "Trombone.Rhythm.Song.MapA");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapB, "Trombone.Rhythm.Song.MapB");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapC, "Trombone.Rhythm.Song.MapC");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapD, "Trombone.Rhythm.Song.MapD");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapT, "Trombone.Rhythm.Song.MapT");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapE, "Trombone.Rhythm.Song.MapE");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_MapF, "Trombone.Rhythm.Song.MapF");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_EasyMapA, "Trombone.Rhythm.Song.EasyMapA");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_EasyMapB, "Trombone.Rhythm.Song.EasyMapB");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Rhythm_Song_EasyMapC, "Trombone.Rhythm.Song.EasyMapC");
	

	//GameplayEffects
	UE_DEFINE_GAMEPLAY_TAG(Trombone_Buff_Speed, "Trombone.Buff.Speed");

	// UI layers
	UE_DEFINE_GAMEPLAY_TAG(Trombone_UI_Layer_Base,		"Trombone.UI.Layer.Base");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_UI_Layer_Popup,		"Trombone.UI.Layer.Popup");
	UE_DEFINE_GAMEPLAY_TAG(Trombone_UI_Layer_Overlay,	"Trombone.UI.Layer.Overlay");
}