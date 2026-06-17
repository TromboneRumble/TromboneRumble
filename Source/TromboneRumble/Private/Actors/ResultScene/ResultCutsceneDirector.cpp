// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ResultScene/ResultCutsceneDirector.h"
#include "AkGameplayStatics.h"
#include "Framework/InGameState.h"
#include "Actors/ResultScene/PodiumActor.h"
#include "Camera/CameraActor.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Blueprint/UserWidget.h"
#include "Framework/TromboneGameInstance.h"
#include "Subsystems/GameStateSubsystem.h"
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
	if (CachedLocalPlayerRankIndex == -1) return;

	if (ZoomSequences.IsValidIndex(CachedLocalPlayerRankIndex - 1) && ZoomSequences[CachedLocalPlayerRankIndex - 1])
	{
		if (!ZoomSequencePlayer)
		{
			ALevelSequenceActor* OutActor;
			ZoomSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), ZoomSequences[CachedLocalPlayerRankIndex - 1], FMovieSceneSequencePlaybackSettings(), OutActor);
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
	
	for (APodiumActor* Podium : PrePlacedPodiums)
	{
		if (Podium)
		{
			Podium->SetActorHiddenInGame(true);
		}
	}
	
	if (const UGameStateSubsystem* GameStateSubsystem = GetGameInstance()->GetSubsystem<UGameStateSubsystem>())
	{
		if (GameStateSubsystem->GetLevelState() == ELevelType::ResultScene)
		{
			PlayResultCutscene();
		}
	}
}

void AResultCutsceneDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!GetWorld())
	{
		return;
	}
	
	Super::EndPlay(EndPlayReason);
}

void HideActorRecursive(AActor* TargetActor, bool bHidden)
{
	if (!TargetActor) return;

	// 본인 숨기기
	TargetActor->SetActorHiddenInGame(bHidden);

	// 부착된 모든 자식 액터들을 가져와서 동일하게 적용
	TArray<AActor*> AttachedActors;
	TargetActor->GetAttachedActors(AttachedActors);

	for (AActor* ChildActor : AttachedActors)
	{
		HideActorRecursive(ChildActor, bHidden);
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
	for (APodiumActor* Podium : PrePlacedPodiums)
	{
		if (Podium && !Podium->IsHidden())
		{
			Podium->SetNameWidgetVisibility(true);
		}
	}
}

void AResultCutsceneDirector::PlayResultCutscene()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC) return;
	
	if (APawn* CurrentPawn = PC->GetPawn())
	{
		PC->DisableInput(PC);
		CurrentPawn->SetActorHiddenInGame(true);
	}

	if (CutsceneCamera)
	{
		PC->SetViewTargetWithBlend(CutsceneCamera, 0.0f);
	}

	// result data : podium actor enabled & skin color
	UTromboneGameInstance* GI = GetGameInstance<UTromboneGameInstance>();
	CachedLocalPlayerRankIndex = GI->GetLocalPlayerRank();
	
	TArray<FPlayerResultSceneData> ResultData = GI->CachedResultSceneData;
	ResultData.Sort();
	
	for (int32 i = 0; i < PrePlacedPodiums.Num(); ++i)
	{
		APodiumActor* PodiumActor = PrePlacedPodiums[i];
		if (!PodiumActor) continue;

		if (i < ResultData.Num())
		{
			PodiumActor->SetPlayerName(ResultData[i].Nickname);
			HideActorRecursive(PodiumActor, false);
			PodiumActor->ApplySkinColor(ResultData[i].PlayerSkinColor);
		}
		else
		{
			HideActorRecursive(PodiumActor, true);
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
				ResultWidget->SetResultData(GI->GetLocalPlayerResultSceneData(), GI->GetLocalPlayerRank());
				ResultWidget->AddToViewport();

				PC->bShowMouseCursor = false;
				FInputModeGameAndUI InputMode;
				
				InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				InputMode.SetHideCursorDuringCapture(false);
				
				PC->SetInputMode(InputMode);
			}
		}
	}
	else
	{
		OnSequenceFinished();
	}
}
