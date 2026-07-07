// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "MatchMenuBackgroundSwitcher.generated.h"

UCLASS()
class TROMBONERUMBLE_API AMatchMenuBackgroundSwitcher : public AActor
{
	GENERATED_BODY()

protected:
	
	/** 로비 맵 태그 → 배경 서브레벨 매핑. 레벨에 배치된 인스턴스에서 채움 */
	UPROPERTY(EditAnywhere, Category = "Background", meta = (Categories = "Trombone.Maps.Lobby"))
	TMap<FGameplayTag, TSoftObjectPtr<UWorld>> BackgroundLevels;

	UPROPERTY(EditAnywhere, Category = "Background")
	float FadeDuration = 0.3f;

private:
	
	void BindToGameState(AGameStateBase* GameState);

	UFUNCTION()
	void HandleSelectedMapChanged(FGameplayTag NewMapTag);

	void BeginSwitch(const FGameplayTag NewMapTag, const bool bInstant);
	
	void SwapLevels();

	UFUNCTION()
	void HandleNewLevelLoaded();

	FGameplayTag CurrentTag;
	FGameplayTag TargetTag;
	FGameplayTag PendingTag;
	bool bSwitching = false;
	int32 LatentUUID = 0;
	FTimerHandle TimerHandle_FadeOut;
	
protected:
	// ~ Begin AActor Interface
	virtual void BeginPlay() override;
	// ~ End AActor Interface
	
};