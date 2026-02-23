// Fill out your copyright notice in the Description page of Project Settings.

#include "AsyncLoadingScreen.h"
#include "Characters/DefaultPlayerController.h"
#include "Framework/LobbyGameMode.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/InGameMode.h"
#include "Framework/InGameState.h"
#include "UI/UserWidgets/OnScreenIndicator/OSI_RhythmRankWidget.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO : ADefaultPlayerController 정리하기!!!

	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !IsLocalController()) return;

	if (URhythmSubsystem* RhythmSubsystem = GameInstance->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnRhythmGameStateChanged.AddDynamic(this, &ADefaultPlayerController::HandleRhythmGameStateChanged);
	}
	
	UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem) return;
	
	GameStateSubsystem->OnLevelStateChanged.AddDynamic(this, &ADefaultPlayerController::HandleLevelStateChanged);
	HandleLevelStateChanged(GameStateSubsystem->GetLevelState());
	
	if (AInGameState* InGameState = GetWorld()->GetGameState<AInGameState>())
	{
		InGameState->OnInGameStateChanged.AddDynamic(this, &ADefaultPlayerController::HandleInGameStateChanged);
	}

	//LoadingScreen
	FAsyncLoadingScreenModule::OnLoadingScreenFinished().AddUObject(
		this, &ADefaultPlayerController::HandleLoadingScreenFinished);

	//	이미 Lobby 맵 안에 있는데 AsyncLoadingScreen 쪽 이벤트가 안 올 수도 있는 상황(클라가 중간 합류) 대비.
	if (GameStateSubsystem->GetLevelState() == ELevelState::Lobby)
	{
		// 여기서 한 번 직접 호출해 줌.
		// 만약 나중에 실제 OnLoadingScreenFinished가 또 불리면
		// HandleLoadingScreenFinished 안의 bHasNotifiedLoadingFinished 때문에 무시됨.
		HandleLoadingScreenFinished();
	}
	//~LoadingScreen

	if (AInGameState* InGameState = GetWorld()->GetGameState<AInGameState>())
	{
		InGameState->OnPlayerStateAdded.AddDynamic(this, &ThisClass::HandlePlayerStateAdded);
		InGameState->OnPlayerStateRemoved.AddDynamic(this, &ThisClass::HandlePlayerStateRemoved);
		InGameState->OnLeaderChanged.AddDynamic(this, &ThisClass::HandleOnLeaderChanged);
	}

	if (IsLocalController())
	{
		Server_NotifyLoadingFinishedToInGameMode();
	}
}

void ADefaultPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FAsyncLoadingScreenModule::OnLoadingScreenFinished().RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void ADefaultPlayerController::HandlePlayerStateAdded(APlayerState* InPlayerState)
{
	if (!IsLocalController() || !RhythmRankWidgetClass)
	{
		return;
	}

	AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}

	bool bNeedRetryNextFrame = false;

	for (APlayerState* PlayerStateInArray : GameState->PlayerArray)
	{
		// 나 자신 제외
		if (!PlayerStateInArray || PlayerState == PlayerStateInArray)
		{
			continue; 
		}

		// 이미 위젯이 있으면 스킵
		if (PlayerStateToRhythmRankWidgetMap.Contains(PlayerStateInArray))
		{
			continue;
		}

		// 멀티플레이어 환경에서 Pawn이 설정안돼있을수 있음
		APawn* PlayerPawnInArray = PlayerStateInArray->GetPawn();
		if (!PlayerPawnInArray)
		{
			bNeedRetryNextFrame = true;
			continue;
		}

		UOSI_RhythmRankWidget* Widget =	CreateWidget<UOSI_RhythmRankWidget>(this, RhythmRankWidgetClass);
		if (!Widget)
		{
			continue;
		}

		Widget->TargetComponent = PlayerPawnInArray->GetRootComponent();
		PlayerStateToRhythmRankWidgetMap.Add(PlayerStateInArray, Widget);
		//WBP_Rhythm보다 한칸 아래
		Widget->AddToViewport(-1);
	}

	if (bNeedRetryNextFrame && !bRetryTimerRunning)
	{
		bRetryTimerRunning = true;

		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				bRetryTimerRunning = false;
				HandlePlayerStateAdded(nullptr);
			}));
	}
}

void ADefaultPlayerController::HandlePlayerStateRemoved(APlayerState* InPlayerState)
{
	if (!InPlayerState) return;

	if (UOSI_RhythmRankWidget* Widget = PlayerStateToRhythmRankWidgetMap.FindRef(InPlayerState))
	{
		Widget->RemoveFromParent();
	}

	PlayerStateToRhythmRankWidgetMap.Remove(InPlayerState);
}

void ADefaultPlayerController::HandleOnLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader)
{
	if (UOSI_RhythmRankWidget* Widget = PlayerStateToRhythmRankWidgetMap.FindRef(NewLeader))
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (UOSI_RhythmRankWidget* Widget = PlayerStateToRhythmRankWidgetMap.FindRef(OldLeader))
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ADefaultPlayerController::Server_NotifyLoadingScreenFinished_Implementation()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !HasAuthority()) return;

	if (UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>())
	{
		GameStateSubsystem->OnPlayerLoadingScreenFinished.Broadcast(this);
	}
}

void ADefaultPlayerController::HandleLoadingScreenFinished()
{
	if (!IsLocalController()) return;

	// 같은 맵에서 여러 번 호출 방지
	if (bHasNotifiedLoadingFinished) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	bHasNotifiedLoadingFinished = true;

	Server_NotifyLoadingScreenFinished();
}

void ADefaultPlayerController::Server_NotifyLoadingFinishedToInGameMode_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (AInGameMode* InGameMode = World->GetAuthGameMode<AInGameMode>())
		{
			InGameMode->HandlePlayerLoadingFinished(this);
		}
	}
}
