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
	 * Start the next gimmick of the sequence after the gap, when Gimmick is the one whose turn it is. Server only.
	 * A gimmick calls it through AGimmickBase::NotifyEventFinished when one event is over.
	 */
	void HandleEventFinished(const AGimmickBase& Gimmick);

	/** @return The gimmick manager of this world, or null. */
	static AGimmickManager* Find(const UWorld* World);

	/**
	 * Find the config of a gimmick type in the stage data of the manager in this world.
	 * Gimmicks use AGimmickBase::GetConfig, and this is for other actors such as the drunkard NPC.
	 *
	 * @return The config, or null when the world has no manager, no stage data or no config of this type.
	 */
	static const UGimmickConfig* FindConfigInWorld(const UWorld* World, EGimmickType GimmickType);

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

	/** Warn on screen and in the log about missing stage data and about each config no gimmick reads. */
	void ValidateStageData() const;

	/** @return Configs that have neither a gimmick of their type in the level nor a Blueprint to spawn. */
	TArray<const UGimmickConfig*> FindUnusedConfigs() const;

	/** @return Configs of gimmicks that start only in a sequence but are in none, so they never start. */
	TArray<const UGimmickConfig*> FindConfigsWithoutSequence() const;

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
	
	/** Turn on every gimmick when the song sends the Event_Spotlight_Start cue. The song sends it once. */
	UFUNCTION()
	void HandleMusicCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	
	/**
	 * Gimmick settings of this level.
	 * A gimmick without a config here runs with its class defaults.
	 */
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

protected:
	
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
	
};