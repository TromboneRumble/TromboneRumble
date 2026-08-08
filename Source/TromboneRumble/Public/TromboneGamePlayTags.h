// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace TromboneGamePlayTags
{
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN : 전역 GameplayTag 변수 선언 (extern과 동일, 정의는 .cpp에서 UE_DEFINE_GAMEPLAY_TAG로)

	// 게임 흐름에 사용되는 맵들 (인게임 제외 OutGame들)
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_OutGame_MainMenu);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_OutGame_MatchMenu);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_OutGame_Tutorial);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_OutGame_Customize);
	
	// 결과 맵.
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Result_OrchestraStage);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Result_SnowField);

	// 로비 맵. 매치 메뉴에서 플레이어가 선택
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Lobby_OrchestraStage);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Lobby_SnowField);

	// 플레이 가능한 인게임
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_InGame_OrchestraStage);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_InGame_SnowField);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_InGame_JazzBar);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Lobby_JazzBar);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Result_JazzBar);

	// 테스트 맵 (never-cook). 플레이어 노출 X. 게임 흐름과 무관.
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Test_Proto);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Test_MK);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Test_HW);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_Test_MJ);

	//RhythmGame Broadcast Messages
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_OnLayoutChanged);

	//RhythmGame Songs
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_Airplane);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_MapA);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_MapB);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_MapC);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_MapD);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_MapT);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_EasyMapA);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_EasyMapB);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Rhythm_Song_EasyMapC);

	//GameplayEffects
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Buff_Speed);
	
	/** Strings */
	const FString ProjectName = TEXT("Trombone");
	const FString MapsCategory = TEXT("Maps");
	
	const FString OutGameCategory = TEXT("OutGame");
	const FString InGameCategory = TEXT("InGame");
	const FString LobbyCategory = TEXT("Lobby");
	const FString ResultCategory = TEXT("Result");
	const FString TestCategory = TEXT("Test");

	const FString MapsRootPath = ProjectName + TEXT(".") + MapsCategory;

	const FString OutGamePath = MapsRootPath + TEXT(".") + OutGameCategory;
	const FString InGamePath = MapsRootPath + TEXT(".") + InGameCategory;
	const FString LobbyPath = MapsRootPath + TEXT(".") + LobbyCategory;
	const FString ResultPath = MapsRootPath + TEXT(".") + ResultCategory;
	const FString TestPath = MapsRootPath + TEXT(".") + TestCategory;
}