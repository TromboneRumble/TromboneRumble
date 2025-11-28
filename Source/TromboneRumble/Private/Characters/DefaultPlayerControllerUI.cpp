// Fill out your copyright notice in the Description page of Project Settings.

#include "AsyncLoadingScreen.h"
#include "Characters/DefaultPlayerController.h"
#include "Framework/LobbyGameMode.h"
#include "Framework/DefaultPlayerState.h"
#include "Prototype/PT_UIInGame.h"
#include "Subsystems/GameStateSubsystem.h"

ADefaultPlayerController::ADefaultPlayerController()
{
	//TODO : 하드 레퍼런싱에서 BP로 변경
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		InGameUIClass = InGameWidgetClassFinder.Class;
	}
}

void ADefaultPlayerController::ShowInteractionUI(bool bShow) const
{
	if (!InGameUI) return;

	// InGameUI->ShowInteractionHint(bShow);
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !IsLocalController()) return;

	UGameStateSubsystem* GameStateSubsystem = GameInstance->GetSubsystem<UGameStateSubsystem>();
	if (!GameStateSubsystem) return;

	switch (GameStateSubsystem->GetGameState())
	{
		case EGameState::MainMenu:
			break;
		case EGameState::Lobby:
			InitializeLobbyUI();
			break;
		case EGameState::InGame:
			InitializeInGameUI();
			break;
		case EGameState::Invalid:
			break;
	}
	
	GameStateSubsystem->OnGameStateChanged.AddDynamic(this, &ADefaultPlayerController::HandleGameStateChanged);
	HandleGameStateChanged(GameStateSubsystem->GetGameState());
	
	FAsyncLoadingScreenModule::OnLoadingScreenFinished().AddUObject(
		this, &ADefaultPlayerController::HandleLoadingScreenFinished);

	// 2) 🔥 Fallback: 이미 Lobby 맵 안에 있는데
	//    AsyncLoadingScreen 쪽 이벤트가 안 올 수도 있는 상황(클라가 중간 합류) 대비.
	if (GameStateSubsystem->GetGameState() == EGameState::Lobby)
	{
		// 여기서 한 번 직접 호출해 줌.
		// 만약 나중에 실제 OnLoadingScreenFinished가 또 불리면
		// HandleLoadingScreenFinished 안의 bHasNotifiedLoadingFinished 때문에 무시됨.
		HandleLoadingScreenFinished();
	}

}

void ADefaultPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FAsyncLoadingScreenModule::OnLoadingScreenFinished().RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void ADefaultPlayerController::InitializeLobbyUI()
{
	// TODO : Lobby UI Load
}

void ADefaultPlayerController::InitializeInGameUI()
{
	if (!InGameUIClass) return;
	
	InGameUI = CreateWidget<UPT_UIInGame>(GetWorld(), InGameUIClass);
	if (!InGameUI) return;
	
	InGameUI->AddToViewport();
	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	bShowMouseCursor = false;
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
