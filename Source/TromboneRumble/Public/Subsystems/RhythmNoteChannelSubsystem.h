// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RhythmNoteChannelSubsystem.generated.h"

USTRUCT()
struct FNoteChannel
{
	GENERATED_BODY()
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnProgress, float /*Alpha*/);
	DECLARE_MULTICAST_DELEGATE(FOnDespawn);

	FOnProgress OnProgress;
	FOnDespawn  OnDespawn;
};

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API URhythmNoteChannelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	FORCEINLINE void OpenChannel(const FGuid& Id) { Channels.Add(Id); }
	FORCEINLINE void CloseChannel(const FGuid& Id) { Channels.Remove(Id); }

	FORCEINLINE FNoteChannel* GetChannelById(const FGuid& Id) { return Channels.Find(Id); }

	void UpdateProgress(const FGuid& Id, const float InAlpha)
	{
		if (FNoteChannel* NoteChannel = Channels.Find(Id))
		{
			NoteChannel->OnProgress.Broadcast(InAlpha);
		}
	}
	void EmitDespawn(const FGuid& Id)
	{
		if (FNoteChannel* NoteChannel = Channels.Find(Id))
		{
			NoteChannel->OnDespawn.Broadcast();
		}
	}

private:
	TMap<FGuid, FNoteChannel> Channels;
};
