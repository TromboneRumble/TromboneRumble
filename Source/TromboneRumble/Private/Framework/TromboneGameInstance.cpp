// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TromboneGameInstance.h"
#include "EasyOnlineSession.h"
#include "AkGameplayStatics.h"
#include "EasyConfig.h"
#include "EngineUtils.h"

TSubclassOf<UOnlineSession> UTromboneGameInstance::GetOnlineSessionClass()
{
	const UEasyConfig* Config = UEasyConfig::Get();
	return Config->OnlineSessionClass;
}

void UTromboneGameInstance::SaveResultSceneData()
{
	CachedResultSceneData.Empty();

	const APlayerState* LocalPS = GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();
	if (LocalPS == nullptr)
	{
		return;
	}
	
	for (APlayerState* PS : TActorRange<APlayerState>(GetWorld()))
	{
		if (const ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS))
		{
			FPlayerResultSceneData Data;
			Data.Nickname = DPS->GetPlayerName();
			Data.Score = DPS->GetScore();
			Data.SpecificScoreData = DPS->GetScoreData();
			Data.PlayerSkinColor = DPS->GetSkinColor();
			Data.bIsLocalPlayer = (DPS == LocalPS);

			CachedResultSceneData.Add(Data);
		}
	}
}

void UTromboneGameInstance::OnStart()
{
	Super::OnStart();
	InitWWiseEngine();
}

void UTromboneGameInstance::PlayMenuBGM()
{
	if (bIsMenuMusicPlaying || PlayMenuBGMEvents.Num() == 0) return;
	
	TArray<EMenuBGMType> Keys;
	PlayMenuBGMEvents.GetKeys(Keys);
	EMenuBGMType SelectedType = Keys[FMath::RandRange(0, Keys.Num() - 1)];

	if (UAkAudioEvent* PlayEvent = PlayMenuBGMEvents[SelectedType])
	{
		UAkGameplayStatics::PostEvent(PlayEvent, nullptr, 0, FOnAkPostEventCallback());
		CurrentMenuBGMType = SelectedType;
		bIsMenuMusicPlaying = true;
	}
}

void UTromboneGameInstance::StopMenuBGM()
{
	if (CurrentMenuBGMType == EMenuBGMType::None) return;

	if (TObjectPtr<UAkAudioEvent>* StopEventPtr = StopMenuBGMEvents.Find(CurrentMenuBGMType))
	{
		UAkGameplayStatics::PostEvent(*StopEventPtr, nullptr, 0, FOnAkPostEventCallback());
        
		bIsMenuMusicPlaying = false;
		CurrentMenuBGMType = EMenuBGMType::None;
	}
}

const FPlayerResultSceneData& UTromboneGameInstance::GetLocalPlayerResultSceneData()
{
	for (const auto& SingleData : CachedResultSceneData)
	{
		if (SingleData.bIsLocalPlayer)
		{
			return SingleData;
		}
	}
	
	// If there is no local player data, create and return it
	// this situation should not actually occur
	CachedResultSceneData.Add(FPlayerResultSceneData());
	return CachedResultSceneData.Last();
}

int32 UTromboneGameInstance::GetLocalPlayerRank()
{
	CachedResultSceneData.Sort();
	
	for (int Rank = 0; Rank < CachedResultSceneData.Num(); Rank++)
	{
		if (CachedResultSceneData[Rank].bIsLocalPlayer)
		{
			return Rank + 1;
		}
	}
	
	return -1;
}

void UTromboneGameInstance::InitWWiseEngine()
{
	{
		AkMemSettings DefaultMemorySettings;
		AK::MemoryMgr::GetDefaultSettings(DefaultMemorySettings);
		AKRESULT InitResult = AK::MemoryMgr::Init(&DefaultMemorySettings);
	}

	{
		AkStreamMgrSettings DefaultStreamMgrSettings;
		AK::StreamMgr::GetDefaultSettings(DefaultStreamMgrSettings);
		AK::IAkStreamMgr* AkStreamMgr = AK::StreamMgr::Create(DefaultStreamMgrSettings);
	}

	{
		AkInitSettings DefaultInitSettings;
		AK::SoundEngine::GetDefaultInitSettings(DefaultInitSettings);
		AkPlatformInitSettings DefaultPlatformSettings;
		AK::SoundEngine::GetDefaultPlatformInitSettings(DefaultPlatformSettings);
		AKRESULT InitResult = AK::SoundEngine::Init(&DefaultInitSettings, &DefaultPlatformSettings);
	}
}

FText UTromboneGameInstance::GetCommonUIText(const FString& Key) const
{
	if (CommonStringTable.IsNull())
	{
		return FText::FromString(TEXT("String Table Missing"));
	}

	FName TableId = FName(*CommonStringTable.ToSoftObjectPath().GetAssetPathString());

	return FText::FromStringTable(TableId, Key);
}

FText UTromboneGameInstance::GetTutorialUIText(const FString& Key) const
{
	if (TutorialStringTable.IsNull())
	{
		return FText::FromString(TEXT("String Table Missing"));
	}

	FName TableId = FName(*TutorialStringTable.ToSoftObjectPath().GetAssetPathString());

	return FText::FromStringTable(TableId, Key);
}
