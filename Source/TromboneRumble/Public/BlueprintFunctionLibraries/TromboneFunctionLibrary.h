// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "TromboneFunctionLibrary.generated.h"

UCLASS()
class TROMBONERUMBLE_API UTromboneFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/** 
	 * @return 예시: (Trombone.Maps.OutGame.MainMenu) -> "/Game/Maps/OutGame/MainMenu.MainMenu"
	 */
	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static FString GetMapPathByMapTag(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag InMapTag);

	/** 주어진 카테고리 태그의 자식 맵 태그들을 GamePlayMap에서 수집. 반환값은 문자열 정렬
	 * @return 예시: (Trombone.Maps.Lobby) -> { Trombone_Maps_Lobby_OrchestraStage, Trombone_Maps_Lobby_SnowField }
	 */
	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static TArray<FGameplayTag> GetMapTagsUnderCategory(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag CategoryTag);

	/**
	 * SourceTag와 같은 테마(leaf)를 가진 TargetCategory의 맵 태그를 반환
	 * @return 예시: (Trombone.Maps.Lobby.OrchestraStage, Trombone.Maps.InGame) → Trombone.Maps.InGame.OrchestraStage
	 */
	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static FGameplayTag GetSiblingMapTag(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag SourceTag, UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag TargetCategoryTag);

	/** 로비 맵 태그를 같은 테마의 인게임 맵 태그로 변환
	 * @return 예시: (Trombone.Maps.Lobby.OrchestraStage) -> Trombone.Maps.InGame.OrchestraStage
	 */
	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static FGameplayTag LobbyToInGameTag(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag LobbyTag);

	/** Randomly select a rhythm song tag that matches the given in-game map */
	UFUNCTION(BlueprintPure, Category = "Rhythm")
	static FGameplayTag PickRandomSongForMap(UPARAM(meta = (Categories = "Trombone.Maps.InGame")) FGameplayTag InGameMapTag);

public:

	UFUNCTION(BlueprintCallable, Category = "Debug",
		meta = (
			DisplayName = "Print Debug",
			DevelopmentOnly,
			AdvancedDisplay = "InKey,Color,Duration,bLog",
			CPP_Default_InKey = "-1",
			CPP_Default_Duration = "7.0",
			CPP_Default_Color = "(R=1.0,G=1.0,B=1.0,A=1.0)",
			CPP_Default_bRandomColor = "true",
			CPP_Default_bLog = "true"
			))
	static void PrintDebug(const FString& Msg, int32 InKey, FLinearColor Color, float Duration, bool bRandomColor, bool bLog);
};
