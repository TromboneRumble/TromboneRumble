// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
#include "Framework/InGameState.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "GimmickManager.generated.h"

class AGimmickBase;
class UGimmickConfig;
class UStageGimmickData;

UCLASS()
class TROMBONERUMBLE_API AGimmickManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	/** Turn on the gimmick of this type, if the level has one. */
	void ActivateGimmick(EGimmickType GimmickType);
	
	/** Turn off the gimmick of this type, if the level has one. */
	void DeactivateGimmick(EGimmickType GimmickType);
	
	/** Turn on every gimmick in the level that is still off. */
	void ActivateAllGimmicks();
	
	/** Turn off every gimmick in the level. */
	void DeactivateAllGimmicks();
	
	/** Start the gimmick of this type now, and turn it on first when it is off. */
	void ForceTriggerGimmick(EGimmickType GimmickType);

	/** @return Gimmick settings of this level, or null when none is set. */
	const UStageGimmickData* GetStageData() const { return StageData; }

	/**
	 * Collect the problems of the stage data that only the level shows: a level with gimmicks but no stage data, and entries no gimmick of the level reads.
	 * UStageGimmickData::IsDataValid checks the rest, which the asset alone shows.
	 * The map check, the warnings at the start of play and the gimmick settings panel all read this.
	 *
	 * @param OutIssues One warning per problem, in the words a designer reads.
	 */
	void CollectLevelIssues(TArray<FText>& OutIssues) const;

	/**
	 * Start the next gimmick of the sequence after the gap, when Gimmick is the one whose turn it is. Server only.
	 * A gimmick calls it through AGimmickBase::NotifyEventFinished when one event is over.
	 */
	void HandleEventFinished(const AGimmickBase& Gimmick);

	/** @return Whether the song has sent the Event_Spotlight_Fever cue in this round. Server only. */
	bool IsFeverTime() const { return bIsFeverTime; }

	/**
	 * Start fever time. The Event_Spotlight_Fever cue calls it, and so does the Trombone.Gimmick.Fever command. Server only.
	 * A gimmick that is running goes on until it ends.
	 * Each sequence with a fever order uses it from the next turn.
	 */
	void BeginFeverTime();

	/** Called on the server when fever time starts. */
	FSimpleMulticastDelegate OnFeverTimeStarted;

	/** @return The gimmick manager of this world, or null. */
	static AGimmickManager* Find(const UWorld* World);

	/**
	 * Find the config of a gimmick type in the stage data of the manager in this world.
	 * Gimmicks use AGimmickBase::GetConfig. An object that reads its config often keeps an FGimmickConfigCache,
	 * and one that reads it now and then, or once when it spawns, uses FindConfigInWorldOrDefault.
	 *
	 * @return The config, or null when the world has no manager, no stage data or no config of this type.
	 */
	static const UGimmickConfig* FindConfigInWorld(const UWorld* World, EGimmickType GimmickType);

	/** @return Config of this type in the stage data of the world, or the defaults of TConfig when the level has none. Never null. */
	template <typename TConfig>
	static const TConfig& FindConfigInWorldOrDefault(const UWorld* World, const EGimmickType GimmickType)
	{
		const TConfig* Config = Cast<TConfig>(FindConfigInWorld(World, GimmickType));
		return Config ? *Config : *GetDefault<TConfig>();
	}

#if WITH_EDITOR
	//~ Begin AActor Interface
	virtual void CheckForErrors() override;
	//~ End AActor Interface
#endif

private:

	/** Spawn, on the server, the gimmick Blueprint of each config whose type is not in the level. */
	void SpawnMissingGimmicks();

	/** Collect every gimmick of the level by type, where the last one wins when two share a type. */
	void RegisterLevelGimmicks();

	/** Warn on screen and in the log about the level problems, and in the editor about the errors of the stage data too. */
	void ValidateStageData() const;

	/** @return The registered gimmick of this type, or null when the level has none. */
	AGimmickBase* FindGimmick(EGimmickType GimmickType) const;

	/** @return Type of every gimmick actor in the world, registered or not. */
	TSet<EGimmickType> FindGimmickTypesInLevel() const;

	/** Start the timer of the first turn of every sequence. Server only. */
	void StartSequences();

	/** Clear every sequence timer and forget where each sequence was. */
	void StopSequences();

	/** Start the gimmick whose turn it is in this run, then move the run to the next turn. */
	void StartNextInSequence(int32 RunIndex);

	/** Wait the gap of this run, then start its next turn. */
	void ScheduleNextInSequence(int32 RunIndex, float Delay);
	
	/** Bind to the game state when it is set after BeginPlay. */
	void HandleGameStateSet(AGameStateBase* NewGameState);
	
	/** Turn every gimmick off when the round ends. */
	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);
	
	/**
	 * Turn on every gimmick on the Event_Spotlight_Start cue, and start fever time on the Event_Spotlight_Fever cue.
	 * The song sends each cue once.
	 */
	UFUNCTION()
	void HandleMusicCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	
	/** 이 레벨이 쓰는 레벨 기믹 설정 에셋. 비우면 기믹이 기본값으로 동작하고, 투척물과 선물, 기믹 시퀀스가 필요한 기믹은 나오지 않는다 */
	UPROPERTY(EditInstanceOnly, Category = "Config", meta = (DisplayName = "레벨 기믹 설정"))
	TObjectPtr<UStageGimmickData> StageData;

	/** Gimmicks of the level by type, filled by RegisterLevelGimmicks. */
	UPROPERTY(VisibleAnywhere, Category = "Config")
	TMap<EGimmickType, TObjectPtr<AGimmickBase>> ManagedGimmicks;

	/** Where one sequence of the stage data is right now. Server only. */
	struct FSequenceRun
	{
		/** Index in UStageGimmickData::GetSequences. */
		int32 SequenceIndex = INDEX_NONE;

		/** Index in Order of the gimmick that starts next. */
		int32 NextIndex = 0;

		/** Gimmick whose turn it is. None while the run waits for the gap. */
		EGimmickType Running = EGimmickType::None;

		FTimerHandle TimerHandle;
	};

	/** One run per sequence of the stage data, filled by StartSequences. */
	TArray<FSequenceRun> SequenceRuns;

	/** Has fever time started in this round. */
	bool bIsFeverTime = false;

protected:
	
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
	
};