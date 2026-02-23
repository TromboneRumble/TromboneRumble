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
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
	}
	else if (AMatchMenuGameState* MatchMenuGameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		MatchMenuGameState->UpdatePlayerList();
	}
}

void ADefaultPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	//서버에서 값이 복제되어왔을때는 숫자만 동기화
	OnLocalScoreChanged.Broadcast(this, 0, EScoreType::None);
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

void ADefaultPlayerState::AddScore(int32 Amount, EScoreType ScoreType)
{
	if (Amount == 0) return;

	// 로컬 점수 선행 계산 후 즉시 갱신
	const float NewScore = GetScore() + static_cast<float>(Amount);
	SetScore(NewScore);
	OnLocalScoreChanged.Broadcast(this, Amount, ScoreType);

	// 서버 동기화
	// Replication은 Server->Client로 이루어지기 때문에 호출 필요
	if (!HasAuthority())
	{
		Server_AddScore(Amount, ScoreType);
	}
}

void ADefaultPlayerState::Server_AddScore_Implementation(int32 Amount, EScoreType ScoreType)
{
	AddScore(Amount, ScoreType);
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