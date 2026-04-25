// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "VoiceChatSubsystem.generated.h"

class APlayerState;

UENUM(BlueprintType)
enum class EVoiceTalkMode : uint8
{
	PushToTalk UMETA(DisplayName = "Push To Talk"),
	AutoVoice  UMETA(DisplayName = "Auto Voice")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTalkModeChanged, EVoiceTalkMode, NewMode);

/**
 * Per-local-player voice chat state: talk mode (PTT/Auto), per-remote mute set, per-remote volume.
 * Persists across map transitions (e.g. MatchMenuMap → InGame).
 */
UCLASS()
class TROMBONERUMBLE_API UVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetTalkMode(EVoiceTalkMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Voice")
	EVoiceTalkMode GetTalkMode() const { return TalkMode; }

	/** Called by the player controller when PTT is pressed (or when Auto mode is entered). */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void BeginLocalTalk();

	/** Called when PTT is released (no-op in Auto mode). */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void EndLocalTalk();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetRemotePlayerMuted(APlayerState* PS, bool bMuted);

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsRemotePlayerMuted(APlayerState* PS) const;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetRemotePlayerVolume(APlayerState* PS, float Volume);

	UFUNCTION(BlueprintPure, Category = "Voice")
	float GetRemotePlayerVolume(APlayerState* PS) const;

	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnVoiceTalkModeChanged OnTalkModeChanged;

private:
	class APlayerController* GetOwningPlayerController() const;
	void ApplyNetworkedVoice(bool bActive);
	void ApplyVolumeToTalker(APlayerState* PS, float Volume);

	/** Ensures this local player is registered with the OSS voice interface so the engine's
	 *  voice-capture device acquires an OwningUserIndex. Without this, StartLocalVoiceProcessing
	 *  fails with ONLINE_FAIL (0xFFFFFFFF) because IsOwningUser() is false. */
	void EnsureLocalTalkerRegistered();

	bool bLocalTalkerRegistered = false;

	UPROPERTY()
	EVoiceTalkMode TalkMode = EVoiceTalkMode::PushToTalk;

	UPROPERTY()
	TSet<TWeakObjectPtr<APlayerState>> MutedPlayers;

	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, float> VolumeMap;
};
