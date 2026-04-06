#include "Framework/LobbyGameState.h"
#include "Framework/TromboneGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Utilities/Defines.h"

ALobbyGameState::ALobbyGameState()
{
	CurrentLobbyState = ELobbyState::None;
	PreviousLobbyState = ELobbyState::None;
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentLobbyState);
	DOREPLIFETIME(ThisClass, SelectedSongTag);
}

void ALobbyGameState::SetLobbyState(const ELobbyState NewState)
{
	if (!HasAuthority() || CurrentLobbyState == NewState) return;

	PreviousLobbyState = CurrentLobbyState;
	CurrentLobbyState = NewState;
	OnRep_LobbyState();
}

void ALobbyGameState::SetSelectedSongTag(const FGameplayTag& InTag)
{
	if (!HasAuthority()) return;
	SelectedSongTag = InTag;
	UE_LOG(LogTemp, Warning, TEXT("Selected Song Tag changed to: %s"), *InTag.ToString());
	OnRep_SelectedSongTag();
}

void ALobbyGameState::OnRep_LobbyState() const
{
	OnLobbyStateChanged.Broadcast(CurrentLobbyState);
}

void ALobbyGameState::OnRep_SelectedSongTag()
{
	if (UTromboneGameInstance* GameInstance = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		GameInstance->SetSelectedSongTag(SelectedSongTag);
	}

	if (UGameDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UGameDataSubsystem>())
	{
		DataSubsystem->PreloadSongAssets(SelectedSongTag);
	}
}
