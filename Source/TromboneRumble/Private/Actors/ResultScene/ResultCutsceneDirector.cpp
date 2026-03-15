// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ResultScene/ResultCutsceneDirector.h"

#include "AkGameplayStatics.h"
#include "Framework/InGameState.h"
#include "Framework/DefaultPlayerState.h"
#include "Actors/ResultScene/PodiumActor.h" // APodiumActor 헤더 경로에 맞게 수정해주세요
#include "Camera/CameraActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidgets/InGame/InGameResultWidget.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"

AResultCutsceneDirector::AResultCutsceneDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AResultCutsceneDirector::SkipResultSequence()
{
	if (SequencePlayer && SequencePlayer->IsPlaying())
	{
		FMovieSceneSequencePlaybackParams Params(SequencePlayer->GetDuration().Time, EUpdatePositionMethod::Jump);
		SequencePlayer->SetPlaybackPosition(Params);
		SequencePlayer->Stop();
		OnSequenceFinished();
	}
}

void AResultCutsceneDirector::PlayZoomSequence(bool bForward)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AInGameState* GameState = Cast<AInGameState>(GetWorld()->GetGameState());
	if (!PC || !GameState) return;


	if (APlayerState* LocalPS = PC->PlayerState)
	{
		int32 Rank = GameState->GetPlayerRank(LocalPS);
		int32 RankIndex = Rank - 1;

		if (ZoomSequences.IsValidIndex(RankIndex) && ZoomSequences[RankIndex])
		{
			if (!ZoomSequencePlayer)
			{
				ALevelSequenceActor* OutActor;
				ZoomSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), ZoomSequences[RankIndex], FMovieSceneSequencePlaybackSettings(), OutActor);
			}

			if (bForward)
			{
				ZoomSequencePlayer->Play();
			}
			else
			{
				ZoomSequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(ZoomSequencePlayer->GetDuration().Time, EUpdatePositionMethod::Jump));
				ZoomSequencePlayer->PlayReverse();
			}
		}
	}
}

void AResultCutsceneDirector::StopBGM()
{
	if (RankingPlayingID != 0 && RankingPlayingID != AK_INVALID_PLAYING_ID)
	{
		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Stop, RankingPlayingID);
		}
	}
	
}

void AResultCutsceneDirector::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (AInGameState* GameState = Cast<AInGameState>(GetWorld()->GetGameState()))
		{
			GameState->OnInGameStateChanged.AddDynamic(this, &AResultCutsceneDirector::HandleInGameStateChanged);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::BindToInGameState);
		}
	}
	

	for (APodiumActor* Podium : PrePlacedPodiums)
	{
		if (Podium)
		{
			Podium->SetActorHiddenInGame(true);
		}
	}
}

void AResultCutsceneDirector::BindToInGameState(AGameStateBase* NewGameState)
{
	if (AInGameState* GameState = Cast<AInGameState>(NewGameState))
	{
		GameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
	}
}

void AResultCutsceneDirector::HandleInGameStateChanged(EInGameState NewState)
{
	if (NewState != EInGameState::End) return;

	AInGameState* GameState = Cast<AInGameState>(GetWorld()->GetGameState());
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!GameState || !PC) return;

	if (APawn* CurrentPawn = PC->GetPawn())
	{
		PC->DisableInput(PC);
		CurrentPawn->SetActorHiddenInGame(true);
	}

	if (CutsceneCamera)
	{
		PC->SetViewTargetWithBlend(CutsceneCamera, 0.0f);
	}

	//플레이어 수에 따라서 색깔 설정 및 보이게 하기
	TArray<APlayerState*> SortedPlayers;
	GameState->GetPlayersSortedByScore(SortedPlayers);

	for (int32 i = 0; i < PrePlacedPodiums.Num(); ++i)
	{
		APodiumActor* PodiumActor = PrePlacedPodiums[i];
		if (!PodiumActor) continue;

		// 실제 플레이어가 존재하는 순위인 경우
		if (i < SortedPlayers.Num())
		{
			if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(SortedPlayers[i]))
			{
				PodiumActor->ApplySkinColor(DefaultPS->GetSkinColor());
				PodiumActor->SetActorHiddenInGame(false);
			}
		}
		else
		{
			PodiumActor->SetActorHiddenInGame(true);
		}
	}

	// 레벨 시퀀스 재생
	if (ResultSequence)
	{
		ALevelSequenceActor* OutActor;
		FMovieSceneSequencePlaybackSettings Settings;

		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), ResultSequence, Settings, OutActor);

		if (SequencePlayer)
		{
			SequencePlayer->OnFinished.AddDynamic(this, &AResultCutsceneDirector::OnSequenceFinished);
			SequencePlayer->Play();
		}
		if (RankingBGM)
		{
			RankingPlayingID = UAkGameplayStatics::PostEvent(RankingBGM, nullptr, 0, FOnAkPostEventCallback());
		}

		if (ResultWidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, ResultWidgetClass);
			if (UInGameResultWidget* ResultWidget = Cast<UInGameResultWidget>(Widget))
			{
				CachedResultWidget = ResultWidget;
				ResultWidget->SetDirector(this);
				ResultWidget->SetResultData(Cast<ADefaultPlayerState>(PC->PlayerState), GameState->GetPlayerRank(PC->PlayerState));
				ResultWidget->AddToViewport();

				PC->bShowMouseCursor = false;
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
	}
	else
	{
		OnSequenceFinished();
	}
}

void AResultCutsceneDirector::OnSequenceFinished()
{
	if (CachedResultWidget.IsValid())
	{
		CachedResultWidget->HideSkipButtonAndShowButtons();
	}
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->bShowMouseCursor = true;
	}
}


