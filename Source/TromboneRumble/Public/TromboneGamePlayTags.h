#pragma once

#include "NativeGameplayTags.h"

namespace TromboneGamePlayTags
{
	// FRONTENDUI_API : 이 변수를 모듈 밖에서도 사용할 수 있도록 export/import 처리 (다른 모듈에서 접근 가능)
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN : 전역 GameplayTag 변수 선언 (extern과 동일, 정의는 .cpp에서 UE_DEFINE_GAMEPLAY_TAG로)

	//TromboneMaps
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_ProtoTypeMainMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_ProtoTypeInGameMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_MainMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_LobbyMap);
	TROMBONERUMBLE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trombone_Maps_InGameMap);
}