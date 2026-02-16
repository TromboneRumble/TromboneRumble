// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/DefaultPlayerState.h"
#include "Characters/TromboneCharacterBase.h"
#include "Framework/LobbyGameState.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Net/UnrealNetwork.h"

ADefaultPlayerState::ADefaultPlayerState()
{
	bReplicates = true;
}


void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquippedWeaponClass);
	DOREPLIFETIME(ThisClass, SkinColor);
	DOREPLIFETIME(ThisClass, CurrentCombo);
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (AMatchMenuGameState* MatchMenuGameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGameState->UpdatePlayerList();
	}
}

void ADefaultPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	OnLocalScoreChanged.Broadcast(this);
}

void ADefaultPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
	{
		DefaultPS->EquippedWeaponClass = this->EquippedWeaponClass;
		DefaultPS->SkinColor = this->SkinColor;
	}
}

void ADefaultPlayerState::AddScore(int32 Amount)
{
	if (!HasAuthority() || Amount == 0)	return;
	const float NewScore = GetScore() + static_cast<float>(Amount);
	SetScore(NewScore);
	OnLocalScoreChanged.Broadcast(this);
}

void ADefaultPlayerState::Server_AddScore_Implementation(int32 Amount)
{
	AddScore(Amount);
}

void ADefaultPlayerState::SetSkinColor(const FLinearColor& InSkinColor)
{
	SkinColor = InSkinColor;
	OnRep_SkinColor();
}

void ADefaultPlayerState::OnRep_SkinColor()
{
	if (APawn* Pawn = GetPawn())
	{
		if (const ATromboneCharacterBase* TromboneCharacter = Cast<ATromboneCharacterBase>(Pawn))
		{
			TromboneCharacter->ApplySkinColor(SkinColor);
		}
	}
}

void ADefaultPlayerState::HandleCombo(ENoteResult InResult)
{
	if (InResult == ENoteResult::Bad || InResult == ENoteResult::None)
	{
		CurrentCombo = 0;
	}
	else
	{
		++CurrentCombo;
	}

	OnComboChanged.Broadcast(InResult, CurrentCombo);
}