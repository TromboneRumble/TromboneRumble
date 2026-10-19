// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/TromboneGameInstance.h"
#include "AkGameplayStatics.h"
#include "EasySessionSubsystem.h"
#include "TromboneGamePlayTags.h"
#include "Subsystems/ToastSubsystem.h"

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

void UTromboneGameInstance::Init()
{
	Super::Init();

	if (UToastSubsystem* Toasts = GetSubsystem<UToastSubsystem>())
	{
		Toasts->OnCollectToastsAfterTravel.AddUObject(this, &ThisClass::QueueConnectionLostNotice);
	}
}

void UTromboneGameInstance::QueueConnectionLostNotice()
{
	// EasySession records why a session ended before it sends the player to the menu, and hands the record out once
	UEasySessionSubsystem* Sessions = GetSubsystem<UEasySessionSubsystem>();
	if (Sessions && Sessions->ConsumePendingDisconnectInfo().Reason != EEasyDisconnectReason::None)
	{
		UToastSubsystem::ShowToastAfterTravel(this, TromboneGamePlayTags::Trombone_UI_Notice_ConnectionLost);
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
