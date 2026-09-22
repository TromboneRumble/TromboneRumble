// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
#include "Framework/InGameState.h"
#include "GameFramework/Actor.h"
#include "GimmickManager.generated.h"

class AGimmickBase;
class UGimmickConfig;
class UStageGimmickData;
enum class EGimmickType : uint8;

UCLASS()
class TROMBONERUMBLE_API AGimmickManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	/** Turn on the gimmick of this type, if the level has one. */
	void ActivateGimmick(EGimmickType GimmickType);
	
	/** Turn off the gimmick of this type, if the level has one. */
	void DeactivateGimmick(EGimmickType GimmickType);
	
	/** Turn on every gimmick in the level. */
	void ActivateAllGimmicks();
	
	/** Turn off every gimmick in the level. */
	void DeactivateAllGimmicks();
	
	/** Start the gimmick of this type now, and turn it on first when it is off. */
	void ForceTriggerGimmick(EGimmickType GimmickType);

	/** @return Gimmick settings of this level, or null when none is set. */
	const UStageGimmickData* GetStageData() const { return StageData; }

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

	/** @return The registered gimmick of this type, or null when the level has none. */
	AGimmickBase* FindGimmick(EGimmickType GimmickType) const;

	/** @return Type of every gimmick actor in the world, registered or not. */
	TSet<EGimmickType> FindGimmickTypesInLevel() const;
	
	/** Bind to the game state when it is set after BeginPlay. */
	void HandleGameStateSet(AGameStateBase* NewGameState);
	
	/** Turn every gimmick off when the round ends. */
	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);
	
	/** Restart every gimmick when the song sends the Event_Spotlight_Start cue. */
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

protected:
	
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
	
};