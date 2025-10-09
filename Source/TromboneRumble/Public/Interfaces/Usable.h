// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Usable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUsable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TROMBONERUMBLE_API IUsable
{
	GENERATED_BODY()

	
public:
	// 눌렀을 때(홀드/연사 시작)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Use")
	void StartUse(AActor* OwnerActor);

	// 뗐을 때(홀드 종료)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Use")
	void StopUse(AActor* OwnerActor);
};
