// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "framework/InGameState.h"
#include "GameFramework/Actor.h"
#include "GimmickManager.generated.h"

class AGimmickBase;
enum class EGimmickType : uint8;

UCLASS()
class TROMBONERUMBLE_API AGimmickManager : public AActor
{
	GENERATED_BODY()
	
public:	
	void ActivateGimmickByType(EGimmickType GimmickType);
	void DeactivateGimmickByType(EGimmickType GimmickType);
	
	void ActivateAllGimmicks();
	
	
	bool IsGimmickActive(EGimmickType GimmickType) const;

protected:
	virtual void BeginPlay() override;
	
	void FindAndRegisterGimmicks();

	void BindToInGameState(AGameStateBase* NewGameState);

	void DeactivateAllGimmicks();

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);
	
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TMap<EGimmickType, TObjectPtr<AGimmickBase>> ManagedGimmicks;
};