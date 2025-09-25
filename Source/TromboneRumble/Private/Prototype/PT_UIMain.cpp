// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/PT_UIMain.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

#include "ProtoType/PT_TRGameInstance.h"
#include "ProtoType/PT_MainMenuPlayerController.h"
#include "Utilities/DebugHelper.h"
void UPT_UIMain::NativeConstruct()
{
	Super::NativeConstruct();

	HostGameButton->OnClicked.AddDynamic(this, &UPT_UIMain::OnClickHostGame);
	JoinGameButton->OnClicked.AddDynamic(this, &UPT_UIMain::OnClickJoinGame);
	StartButton->OnClicked.AddDynamic(this, &UPT_UIMain::OnClickStartGame);
	StartButton->SetVisibility(ESlateVisibility::Collapsed);

	UPT_TRGameInstance* GI = GetGameInstance<UPT_TRGameInstance>();
	if (GI)
	{
		GI->OnSessionJoined.AddDynamic(this, &UPT_UIMain::HandleSessionJoined);
	}

	const APlayerController* PC = GetOwningPlayer();
	const bool bIsHost = PC && PC->HasAuthority();
	AuthText->SetText(FText::FromString(FString::Printf(TEXT("%s"), bIsHost ? TEXT("Host") : TEXT("Client"))));
}

void UPT_UIMain::HandleSessionJoined(const FString& LobbyCode)
{
	if (!LobbyCodeText) return;
	
	LobbyCodeText->SetText(FText::FromString(LobbyCode));

	const APlayerController* PC = GetOwningPlayer();
	if (PC && PC->HasAuthority())
	{
		StartButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPT_UIMain::OnClickStartGame()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Start Game Clicked")));

	if (APT_MainMenuPlayerController* PC = GetOwningPlayer<APT_MainMenuPlayerController>())
	{
		PC->Server_RequestStartGame();
	}
}

void UPT_UIMain::OnClickHostGame()
{
	if (UPT_TRGameInstance* GI = GetGameInstance<UPT_TRGameInstance>())
	{
		GI->HostSessionWithCode();
	}
	else
	{
		Debug::Print(TEXT("No UPT_TRGameInstance "));
	}
}

void UPT_UIMain::OnClickJoinGame()
{
	if (LobbyCodeInput == nullptr) return;

	const FString LobbyCode = LobbyCodeInput->GetText().ToString();

	if (UPT_TRGameInstance* GI = GetGameInstance<UPT_TRGameInstance>())
	{
		if (LobbyCode.IsEmpty()) return;
		
		GI->FindAndJoinSessionByCode(LobbyCode);
	}
}