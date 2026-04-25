// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/VoiceChatSubsystem.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"

void UVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TalkMode = EVoiceTalkMode::PushToTalk;
	MutedPlayers.Reset();
	VolumeMap.Reset();
}

void UVoiceChatSubsystem::Deinitialize()
{
	EndLocalTalk();
	Super::Deinitialize();
}

APlayerController* UVoiceChatSubsystem::GetOwningPlayerController() const
{
	const ULocalPlayer* LP = GetLocalPlayer();
	return LP ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
}

void UVoiceChatSubsystem::ApplyNetworkedVoice(bool bActive)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->ToggleSpeaking(bActive);
	}
}

void UVoiceChatSubsystem::EnsureLocalTalkerRegistered()
{
	if (bLocalTalkerRegistered)
	{
		return;
	}

	const ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UWorld* World = LP->GetWorld();
	if (!World)
	{
		return;
	}

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (!OSS)
	{
		return;
	}

	IOnlineVoicePtr VoiceInt = OSS->GetVoiceInterface();
	if (!VoiceInt.IsValid())
	{
		UE_LOG(LogTemp, Verbose, TEXT("VoiceChatSubsystem: OSS '%s' has no voice interface (likely no active session yet)"),
			*OSS->GetSubsystemName().ToString());
		return;
	}

	const int32 LocalUserNum = LP->GetControllerId();

	// Registering the local talker sets the voice engine's OwningUserIndex, which is a
	// prerequisite for StartLocalVoiceProcessing to succeed. Safe to call multiple times
	// (FOnlineVoiceImpl guards via FLocalTalker::bIsRegistered).
	const bool bRegistered = VoiceInt->RegisterLocalTalker(LocalUserNum);
	//UE_LOG(LogTemp, Log, TEXT("VoiceChatSubsystem: RegisterLocalTalker(%d) -> %s"),
	//	LocalUserNum, bRegistered ? TEXT("OK") : TEXT("FAIL"));

	bLocalTalkerRegistered = bRegistered;
}

void UVoiceChatSubsystem::SetTalkMode(EVoiceTalkMode NewMode)
{
	if (TalkMode == NewMode)
	{
		return;
	}

	TalkMode = NewMode;
	OnTalkModeChanged.Broadcast(TalkMode);

	if (TalkMode == EVoiceTalkMode::AutoVoice)
	{
		BeginLocalTalk();
	}
	else
	{
		EndLocalTalk();
	}
}

void UVoiceChatSubsystem::BeginLocalTalk()
{
	// First-use registration: the OSS voice engine's OwningUserIndex must be set before
	// StartLocalVoiceProcessing can succeed. EasySessions + Steam login flow does not
	// trigger this automatically, so we do it lazily here.
	EnsureLocalTalkerRegistered();
	ApplyNetworkedVoice(true);
}

void UVoiceChatSubsystem::EndLocalTalk()
{
	ApplyNetworkedVoice(false);
}

void UVoiceChatSubsystem::SetRemotePlayerMuted(APlayerState* PS, bool bMuted)
{
	if (!PS)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();
	if (!NetId.IsValid())
	{
		return;
	}

	if (bMuted)
	{
		MutedPlayers.Add(PS);
		PC->ServerMutePlayer(NetId);
	}
	else
	{
		MutedPlayers.Remove(PS);
		PC->ServerUnmutePlayer(NetId);
	}
}

bool UVoiceChatSubsystem::IsRemotePlayerMuted(APlayerState* PS) const
{
	return PS && MutedPlayers.Contains(PS);
}

void UVoiceChatSubsystem::SetRemotePlayerVolume(APlayerState* PS, float Volume)
{
	if (!PS)
	{
		return;
	}

	const float Clamped = FMath::Clamp(Volume, 0.0f, 1.0f);
	VolumeMap.Add(PS, Clamped);
	ApplyVolumeToTalker(PS, Clamped);
}

float UVoiceChatSubsystem::GetRemotePlayerVolume(APlayerState* PS) const
{
	if (!PS)
	{
		return 1.0f;
	}
	if (const float* Found = VolumeMap.Find(PS))
	{
		return *Found;
	}
	return 1.0f;
}

void UVoiceChatSubsystem::ApplyVolumeToTalker(APlayerState* PS, float Volume)
{
	if (!PS)
	{
		return;
	}

	const APawn* OwnerPawn = PS->GetPawn();
	if (!OwnerPawn)
	{
		return;
	}

	if (UTromboneVOIPTalker* Talker = OwnerPawn->FindComponentByClass<UTromboneVOIPTalker>())
	{
		Talker->SetVolumeMultiplier(Volume);
	}
}
