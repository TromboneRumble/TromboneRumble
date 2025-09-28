#pragma once

#include "NativeGameplayTags.h"

namespace TromboneGamePlayTags
{
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN : 전역 GameplayTag 변수 선언 (extern과 동일, 정의는 .cpp에서 UE_DEFINE_GAMEPLAY_TAG로)

	//TromboneMaps
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_ProtoTypeMainMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_ProtoTypeInGameMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_MainMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_LobbyMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_InGameMap);
}