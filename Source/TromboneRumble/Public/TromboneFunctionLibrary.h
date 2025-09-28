// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "TromboneFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static FString GetMapPathByTag(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag InMapTag);

	UFUNCTION(BlueprintPure, Category = "GameMaps")
	static FName GetMapPackageNameByTag(UPARAM(meta = (Categories = "Trombone.Maps")) FGameplayTag InMapTag);

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
