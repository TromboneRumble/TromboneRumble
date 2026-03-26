// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	void BindToInGameState(AGameStateBase* NewGameState);

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState NewState);

	UFUNCTION()
	void OnSequenceFinished();

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

};
