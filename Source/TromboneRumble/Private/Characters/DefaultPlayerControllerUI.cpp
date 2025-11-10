// Fill out your copyright notice in the Description page of Project Settings.

#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
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

	InGameUI->ShowInteractionHint(bShow);
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	const UGameInstance* GI = GetGameInstance();
	if (!GI || !IsLocalController()) return;

	UGameStateSubsystem* GameStateSubsystem = GI->GetSubsystem<UGameStateSubsystem>();
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
	
	Server_NotifyClientReady();

	if (TestSoundEvent)
	{
		FOnAkPostEventCallback OnCallback;
		UAkGameplayStatics::PostEvent(TestSoundEvent, this, AK_EndOfEvent, OnCallback);
	}
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

void ADefaultPlayerController::Server_NotifyClientReady_Implementation()
{
	ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (!PS) return;

	PS->SetIsReady(true);
	
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->NotifyClientReady(this);
	}
}