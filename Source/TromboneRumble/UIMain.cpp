// Fill out your copyright notice in the Description page of Project Settings.

#include "UIMain.h"
#include "TRGameInstance.h"
#include "MainMenuPlayerController.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void UUIMain::NativeConstruct()
{
	Super::NativeConstruct();

	HostGameButton->OnClicked.AddDynamic(this, &UUIMain::OnClickHostGame);
	JoinGameButton->OnClicked.AddDynamic(this, &UUIMain::OnClickJoinGame);
	StartButton->OnClicked.AddDynamic(this, &UUIMain::OnClickStartGame);
	StartButton->SetVisibility(ESlateVisibility::Collapsed);

	UTRGameInstance* GI = GetGameInstance<UTRGameInstance>();
	if (GI)
	{
		GI->OnSessionJoined.AddDynamic(this, &UUIMain::HandleSessionJoined);
	}

	const APlayerController* PC = GetOwningPlayer();
	const bool bIsHost = PC && PC->HasAuthority();
	AuthText->SetText(FText::FromString(FString::Printf(TEXT("%s"), bIsHost ? TEXT("Host") : TEXT("Client"))));
}

void UUIMain::HandleSessionJoined(const FString& LobbyCode)
{
	if (!LobbyCodeText) return;
	
	LobbyCodeText->SetText(FText::FromString(LobbyCode));

	const APlayerController* PC = GetOwningPlayer();
	if (PC && PC->HasAuthority())
	{
		StartButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUIMain::OnClickStartGame()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Start Game Clicked")));

	if (AMainMenuPlayerController* PC = GetOwningPlayer<AMainMenuPlayerController>())
	{
		PC->Server_RequestStartGame();
	}
}

void UUIMain::OnClickHostGame()
{
	if (UTRGameInstance* GI = GetGameInstance<UTRGameInstance>())
	{
		GI->HostSessionWithCode();
	}
}

void UUIMain::OnClickJoinGame()
{
	if (LobbyCodeInput == nullptr) return;

	const FString LobbyCode = LobbyCodeInput->GetText().ToString();

	if (UTRGameInstance* GI = GetGameInstance<UTRGameInstance>())
	{
		if (LobbyCode.IsEmpty()) return;
		
		GI->FindAndJoinSessionByCode(LobbyCode);
	}
}