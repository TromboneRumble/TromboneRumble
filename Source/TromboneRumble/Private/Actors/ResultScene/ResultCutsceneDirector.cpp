// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ResultScene/ResultCutsceneDirector.h"
#include "Framework/InGameState.h"
#include "Framework/DefaultPlayerState.h"
#include "Actors/ResultScene/PodiumActor.h" // APodiumActor 헤더 경로에 맞게 수정해주세요
#include "Camera/CameraActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidgets/InGame/InGameResultWidget.h"

AResultCutsceneDirector::AResultCutsceneDirector()
{
	PrimaryActorTick.bCanEverTick = false;
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
	}
	else
	{
		OnSequenceFinished();
	}
}

void AResultCutsceneDirector::OnSequenceFinished()
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
			ALevelSequenceActor* OutActor;
			FMovieSceneSequencePlaybackSettings Settings;

			ZoomSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), ZoomSequences[RankIndex], Settings, OutActor);

			if (ZoomSequencePlayer)
			{
				ZoomSequencePlayer->Play();
			}
		}
	}

	if (ResultWidgetClass)
	{
		UUserWidget* ResultWidget = CreateWidget<UUserWidget>(PC, ResultWidgetClass);
		if (ResultWidget)
		{
			if (UInGameResultWidget* InGameWidget = Cast<UInGameResultWidget>(ResultWidget))
			{
				int32 MyRank = GameState->GetPlayerRank(PC->PlayerState);
				InGameWidget->SetResultData(Cast<ADefaultPlayerState>(PC->PlayerState), MyRank);
			}
			ResultWidget->AddToViewport();

			// UI 상호작용을 위해 마우스 커서 표시 및 InputMode 변경
			PC->bShowMouseCursor = true;
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
			PC->SetInputMode(InputMode);
		}
	}
}


