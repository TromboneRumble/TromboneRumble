// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/DefaultPlayerState.h"
#include "Characters/TromboneCharacterBase.h"
#include "Framework/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

ADefaultPlayerState::ADefaultPlayerState()
{
	bIsReady = false;
}

void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsReady);
	DOREPLIFETIME(ThisClass, EquippedInstrumentClass);
	DOREPLIFETIME(ThisClass, SkinColor);
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
}

void ADefaultPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(PlayerState))
	{
		DefaultPS->EquippedInstrumentClass = this->EquippedInstrumentClass;
		DefaultPS->bIsReady = this->bIsReady;
		DefaultPS->SkinColor = this->SkinColor;
	}
}

void ADefaultPlayerState::SetIsReady(bool bReady)
{
	if (!HasAuthority() || bIsReady == bReady) return;
	
	bIsReady = bReady;
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