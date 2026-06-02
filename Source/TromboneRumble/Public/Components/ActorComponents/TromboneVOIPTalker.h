// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "TromboneVOIPTalker.generated.h"

class USoundAttenuation;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTalkingStateChanged, bool, bIsTalking);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UTromboneVOIPTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
	UTromboneVOIPTalker(const FObjectInitializer& ObjectInitializer);

	/** If true, voice plays as 3D positional audio attached to the owning pawn. If false, plays as 2D (omnidirectional). */
	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	bool bPositional = true;

	/** Attenuation used when bPositional=true. */
	UPROPERTY(EditDefaultsOnly, Category = "Voice", meta = (EditCondition = "bPositional"))
	TObjectPtr<USoundAttenuation> AttenuationSettings = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnVoiceTalkingStateChanged OnTalkingStateChanged;

	/** Register this talker with the given PlayerState and apply positional/2D voice settings. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void RegisterTalker(APlayerState* InPlayerState);

	/** Returns true if RegisterRemoteTalker succeeded for the associated PlayerState. */
	bool IsRemoteTalkerRegistered() const { return RegisteredRemoteTalkerId.IsValid(); }

	/** Apply a per-listener volume multiplier (0..1) to voice playback.
	 *  Stored as a pending value and also applied immediately to the cached UAudioComponent
	 *  if the remote player is currently talking. Re-applied on each subsequent OnTalkingBegin. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetVolumeMultiplier(float InVolume);

	//~ Begin UVOIPTalker interface
	virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;
	virtual void OnTalkingEnd() override;
	//~ End UVOIPTalker interface

	//~ Begin UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent interface

private:
	/** Cached audio component provided by the engine via OnTalkingBegin, used to apply volume mid-talk. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAudioComponent> CachedAudioComponent;

	/** Per-listener volume multiplier persisted across talk sessions. */
	float PendingVolumeMultiplier = 1.0f;

	/** Cached NetId we registered as a remote talker in RegisterTalker(), used to unregister on EndPlay. */
	FUniqueNetIdRepl RegisteredRemoteTalkerId;
};
