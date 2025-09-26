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
};
