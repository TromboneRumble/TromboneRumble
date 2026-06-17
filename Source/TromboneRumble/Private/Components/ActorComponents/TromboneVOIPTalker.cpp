// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "Components/AudioComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/VoiceInterface.h"
#include "OnlineSubsystem.h"
#include "Sound/SoundAttenuation.h"
#include "Subsystems/VoiceChatSubsystem.h"

UTromboneVOIPTalker::UTromboneVOIPTalker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTromboneVOIPTalker::RegisterTalker(APlayerState* InPlayerState)
{
	if (!InPlayerState)
	{
		return;
	}

	FVoiceSettings VoiceSettings;
	if (bPositional)
	{
		// 3D playback attached to the owning pawn's root.
		VoiceSettings.ComponentToAttachTo = GetOwner() ? GetOwner()->GetRootComponent() : nullptr;
		VoiceSettings.AttenuationSettings = AttenuationSettings;
	}
	else
	{
		// 2D (omnidirectional) playback — null AttenuationSettings skips spatialization.
		VoiceSettings.ComponentToAttachTo = nullptr;
		VoiceSettings.AttenuationSettings = nullptr;
	}
	Settings = VoiceSettings;

	RegisterWithPlayerState(InPlayerState);

	// Explicitly register this PS as a remote talker with the local voice interface.
	//
	// Why: In Steam OSS this usually happens automatically via
	//   FOnlineSessionSteam::RegisterPlayer -> RegisterVoice -> VoiceInt->RegisterRemoteTalker.
	// In the NULL OSS + EasySessions LAN path (our -nosteam -tr_lan test config), that chain
	// can be skipped depending on join timing — which leaves every remote PS unknown to the
	// listener's voice interface, causing inbound voice packets to be silently dropped and
	// UVOIPTalker::OnTalkingBegin never firing.
	//
	// Manually calling RegisterRemoteTalker here guarantees correct registration regardless
	// of subsystem / session path. For the LOCAL player's own PS the voice interface treats
	// the call as a no-op (IsLocalTalker() short-circuits), so we can call it unconditionally.
	const FUniqueNetIdRepl& NetId = InPlayerState->GetUniqueId();
	if (NetId.IsValid())
	{
		if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
		{
			if (IOnlineVoicePtr VoiceInt = OSS->GetVoiceInterface())
			{
				VoiceInt->RegisterRemoteTalker(*NetId);
				RegisteredRemoteTalkerId = NetId;
			}
		}
	}

	if (ULocalPlayer* LP = GetWorld()->GetFirstLocalPlayerFromController())
	{
		if (UVoiceChatSubsystem* VCS = LP->GetSubsystem<UVoiceChatSubsystem>())
		{
			VCS->ApplyVolumeToTalker(InPlayerState);
		}
	}
}

void UTromboneVOIPTalker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Mirror the RegisterRemoteTalker call from RegisterTalker. Without this, when a pawn
	// is destroyed (player leaves, map transition) the voice interface keeps a stale entry
	// that can block re-registration of a player who re-joins with the same NetId.
	if (RegisteredRemoteTalkerId.IsValid())
	{
		if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
		{
			if (IOnlineVoicePtr VoiceInt = OSS->GetVoiceInterface())
			{
				VoiceInt->UnregisterRemoteTalker(*RegisteredRemoteTalkerId);
			}
		}
		RegisteredRemoteTalkerId = FUniqueNetIdRepl();
	}

	Super::EndPlay(EndPlayReason);
}

void UTromboneVOIPTalker::SetVolumeMultiplier(float InVolume)
{
	PendingVolumeMultiplier = FMath::Clamp(InVolume, 0.0f, 40.0f);

	// If we're currently receiving a talk stream, apply immediately to the live audio component.
	if (UAudioComponent* AudioComp = CachedAudioComponent.Get())
	{
		AudioComp->SetVolumeMultiplier(PendingVolumeMultiplier);
	}
}

void UTromboneVOIPTalker::SetPushToTalkSpeaking(bool bSpeaking)
{
	bPTTSpeaking = bSpeaking;
	UpdateAndBroadcast();
}

void UTromboneVOIPTalker::UpdateAndBroadcast()
{
	const bool bCombined = bPTTSpeaking || bAutoTalking;
	if (bCombined != bLastBroadcast)
	{
		bLastBroadcast = bCombined;
		OnTalkingStateChanged.Broadcast(bCombined);
	}
}

void UTromboneVOIPTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
	Super::OnTalkingBegin(AudioComponent);

	CachedAudioComponent = AudioComponent;
	if (AudioComponent)
	{
		// Re-apply any pending per-listener volume (default 1.0) each time a new talk session starts.
		AudioComponent->SetVolumeMultiplier(PendingVolumeMultiplier);
	}

	bAutoTalking = true;
	UpdateAndBroadcast();
}

void UTromboneVOIPTalker::OnTalkingEnd()
{
	Super::OnTalkingEnd();

	CachedAudioComponent.Reset();
	bAutoTalking = false;
	// While PTT is held, the combined state stays true so the indicator doesn't flicker during pauses.
	UpdateAndBroadcast();
}
