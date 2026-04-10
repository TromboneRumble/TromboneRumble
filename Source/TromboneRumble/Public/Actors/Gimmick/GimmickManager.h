// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
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
	void DeactivateAllGimmicks();
	
	
	bool IsGimmickActive(EGimmickType GimmickType) const;

protected:
	virtual void BeginPlay() override;
	
	void FindAndRegisterGimmicks();

	void BindToInGameState(AGameStateBase* NewGameState);

	

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);

	UFUNCTION()
	void OnMusicCallbackReceived(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TMap<EGimmickType, TObjectPtr<AGimmickBase>> ManagedGimmicks;
};