// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GameMapDeveloperSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Trombone Map Settings"))
class TROMBONERUMBLE_API UGameMapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Config : 해당 멤버 변수를 .ini파일에서 읽고 저장하도록 함. 클래스쪽에 (Config=Game/Engine/Editor)로 표시해야함.
	// ForceInlineRow : 한줄로 표현
	UPROPERTY(Config, EditAnywhere, meta = (ForceInlineRow, Categories = "Trombone.Maps"))
	TMap<FGameplayTag, FSoftObjectPath> GamePlayMap;

	/** Maps left out of the selection list in shipping builds. Other builds still show them. */
	UPROPERTY(Config, EditAnywhere, meta = (Categories = "Trombone.Maps"))
	TArray<FGameplayTag> ShippingHiddenMaps;
};
