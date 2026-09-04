// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TromboneGameInstance.h"
#include "EasyOnlineSession.h"
#include "AkGameplayStatics.h"
#include "EasyConfig.h"

TSubclassOf<UOnlineSession> UTromboneGameInstance::GetOnlineSessionClass()
{
	const UEasyConfig* Config = UEasyConfig::Get();
	return Config->OnlineSessionClass;
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
