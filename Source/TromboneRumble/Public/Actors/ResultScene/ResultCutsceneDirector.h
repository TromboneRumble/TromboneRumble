// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Framework/InGameState.h"
#include "ResultCutsceneDirector.generated.h"

class UInGameResultWidget;
class UAkAudioEvent;
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class ACameraActor;
class APodiumActor;


UCLASS()
class TROMBONERUMBLE_API AResultCutsceneDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	AResultCutsceneDirector();

	void SkipResultSequence();
	void PlayZoomSequence(bool bForward);
	void StopBGM();
	
protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnSequenceFinished();

private:

	/** Loads the background for the current result stage, then starts the cutscene. */
	void LoadBackgroundThenPlayCutscene();

	/** Called once the background sublevel has finished loading. */
	UFUNCTION()
	void HandleBackgroundLoaded();

	void PlayResultCutscene();

	UPROPERTY(EditDefaultsOnly, Category = "Config|UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Config|Cinematic")
	TObjectPtr<ULevelSequence> ResultSequence;

	UPROPERTY(EditAnywhere, Category = "Config|Cinematic")
	TArray<TObjectPtr<ULevelSequence>> ZoomSequences;

	UPROPERTY(EditAnywhere, Category = "Config|Cinematic")
	TObjectPtr<ACameraActor> CutsceneCamera;

	UPROPERTY(EditAnywhere, Category = "Config|Podium")
	TArray<TObjectPtr<APodiumActor>> PrePlacedPodiums;

	UPROPERTY(EditAnywhere, Category = "Config|Sound")
	TObjectPtr<UAkAudioEvent> RankingBGM;

	/** Background sublevel to load for each result stage. Stages left out here start the cutscene right away. */
	UPROPERTY(EditAnywhere, Category = "Config|Background", meta = (ForceInlineRow, Categories = "Trombone.Maps.Result"))
	TMap<FGameplayTag, TSoftObjectPtr<UWorld>> BackgroundLevels;

private:
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> ZoomSequencePlayer;

	UPROPERTY()
	TSoftObjectPtr<UInGameResultWidget> CachedResultWidget;

	UPROPERTY()
	int32 RankingPlayingID = 0;

	int32 CachedLocalPlayerRankIndex = -1;

	/** Ticket number for the level streaming call. Must differ per call. */
	int32 LatentUUID = 0;

};
