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

/** What changes from one result stage to the next. */
USTRUCT()
struct FResultStageSetup
{
	GENERATED_BODY()

	/** Background sublevel loaded before the cutscene starts. */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> BackgroundLevel;

	/** Runs next to the shared result sequence. Put lights that differ per stage in here. */
	UPROPERTY(EditAnywhere)
	TObjectPtr<ULevelSequence> LightingSequence;
};

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

	/** Starts the stage's lighting sequence next to the main one, if it has one. */
	void StartLightingSequence();

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

	/** What changes per stage. Stages left out here start the cutscene right away. */
	UPROPERTY(EditAnywhere, Category = "Config|Stage", meta = (Categories = "Trombone.Maps.Result"))
	TMap<FGameplayTag, FResultStageSetup> StageSetups;

private:
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> SequencePlayer;
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> ZoomSequencePlayer;
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> LightingSequencePlayer;

	UPROPERTY()
	TSoftObjectPtr<UInGameResultWidget> CachedResultWidget;

	UPROPERTY()
	int32 RankingPlayingID = 0;

	int32 CachedLocalPlayerRankIndex = -1;

	/** Ticket number for the level streaming call. Must differ per call. */
	int32 LatentUUID = 0;

	/** Stage picked when the cutscene starts. Used to look up StageSetups later on. */
	FGameplayTag ActiveStageTag;

};
