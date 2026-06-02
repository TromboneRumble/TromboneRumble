// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Gimmick/GimmickManager.h"

#include "EngineUtils.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Subsystems/RhythmSubsystem.h"

void AGimmickManager::ActivateGimmickByType(const EGimmickType GimmickType)
{
	if (const TObjectPtr<AGimmickBase>* FoundGimmick = ManagedGimmicks.Find(GimmickType))
	{
		if (FoundGimmick && *FoundGimmick)
		{
			(*FoundGimmick)->Activate();
		}
	}
}

void AGimmickManager::DeactivateGimmickByType(EGimmickType GimmickType)
{
	if (const TObjectPtr<AGimmickBase>* FoundGimmick = ManagedGimmicks.Find(GimmickType))
	{
		if (FoundGimmick && *FoundGimmick)
		{
			(*FoundGimmick)->Deactivate();
		}
	}
}

void AGimmickManager::ActivateAllGimmicks()
{
	for (auto& Pair : ManagedGimmicks)
	{
		if (Pair.Value)
		{
			Pair.Value->Activate();
		}
	}
}

void AGimmickManager::DeactivateAllGimmicks()
{
	for (auto& Pair : ManagedGimmicks)
	{
		if (Pair.Value)
		{
			Pair.Value->Deactivate();
		}
	}
}

bool AGimmickManager::IsGimmickActive(EGimmickType GimmickType) const
{
	if (const TObjectPtr<AGimmickBase>* FoundGimmick = ManagedGimmicks.Find(GimmickType))
	{
		if (FoundGimmick && *FoundGimmick)
		{
			return (*FoundGimmick)->IsActive();
		}
	}
	return false;
}

void AGimmickManager::BeginPlay()
{
	Super::BeginPlay();
	
	FindAndRegisterGimmicks();

	if (UWorld* World = GetWorld())
	{
		if (AInGameState* GameState = Cast<AInGameState>(GetWorld()->GetGameState()))
		{
			GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::BindToInGameState);
		}
	}
	if (HasAuthority())
	{
		if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RS->OnMusicCallback.AddDynamic(this, &ThisClass::OnMusicCallbackReceived);
		}
	}
}

void AGimmickManager::FindAndRegisterGimmicks()
{
	ManagedGimmicks.Empty();
	
	for (TActorIterator<AGimmickBase> It(GetWorld()); It; ++It)
	{
		AGimmickBase* Gimmick = *It;
		ManagedGimmicks.Add(Gimmick->GetGimmickType(), Gimmick);
	}
}

void AGimmickManager::BindToInGameState(AGameStateBase* NewGameState)
{
	if (AInGameState* GameState = Cast<AInGameState>(NewGameState))
	{
		GameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
	}
}

void AGimmickManager::HandleInGameStateChanged(EInGameState InGameState)
{
	if (InGameState == EInGameState::End)
	{
		DeactivateAllGimmicks();
	}
}

void AGimmickManager::OnMusicCallbackReceived(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (!HasAuthority()) return;
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString& CueString = MusicInfo->UserCueName;
		if (!CueString.IsEmpty())
		{
			const FName CueName(*CueString);
			if (CueName == TEXT("Event_Spotlight_Start"))
			{
				DeactivateAllGimmicks();
				ActivateAllGimmicks();
			}
		}
	}
}