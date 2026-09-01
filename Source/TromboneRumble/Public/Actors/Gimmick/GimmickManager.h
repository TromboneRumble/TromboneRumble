// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
#include "Framework/InGameState.h"
#include "GameFramework/Actor.h"
#include "GimmickManager.generated.h"

class AGimmickBase;
enum class EGimmickType : uint8;

UCLASS()
class TROMBONERUMBLE_API AGimmickManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	/** Turns on the gimmick of this type, if the level has one. */
	void ActivateGimmickByType(EGimmickType GimmickType);
	
	/** Turns off the gimmick of this type, if the level has one. */
	void DeactivateGimmickByType(EGimmickType GimmickType);
	
	/** Turns on every gimmick in the level. */
	void ActivateAllGimmicks();
	
	/** Turns off every gimmick in the level. */
	void DeactivateAllGimmicks();
	
	/** @return true while the gimmick of this type is running. */
	bool IsGimmickActive(EGimmickType GimmickType) const;
	
private:
	
	/** Collects every gimmick in the level by type. With two of the same type, only the last one is kept. */
	void FindAndRegisterGimmicks();
	
	void BindToInGameState(AGameStateBase* NewGameState);
	
	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);
	
	UFUNCTION()
	void OnMusicCallbackReceived(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TMap<EGimmickType, TObjectPtr<AGimmickBase>> ManagedGimmicks;
	
protected:
	
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
	
};