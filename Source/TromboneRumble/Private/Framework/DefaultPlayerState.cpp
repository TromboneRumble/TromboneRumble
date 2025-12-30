// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/DefaultPlayerState.h"
#include "Characters/TromboneCharacterBase.h"
#include "Framework/LobbyGameState.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/DebugHelper.h"

ADefaultPlayerState::ADefaultPlayerState()
{
	bReplicates = true;
}

void ADefaultPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		if (PC->IsLocalController())
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (URhythmSubsystem* RhythmSubsystem = GI->GetSubsystem<URhythmSubsystem>())
				{
					RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ADefaultPlayerState::HandleNoteDetected);
				}
			}
		}
	}
}

void ADefaultPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		if (PC->IsLocalController())
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (URhythmSubsystem* RhythmSubsystem = GI->GetSubsystem<URhythmSubsystem>())
				{
					RhythmSubsystem->OnNoteDetected.RemoveDynamic(this, &ADefaultPlayerState::HandleNoteDetected);
				}
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}


void ADefaultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquippedWeaponClass);
	DOREPLIFETIME(ThisClass, SkinColor);
	DOREPLIFETIME(ThisClass, CurrentCombo);
	DOREPLIFETIME(ThisClass, bLastComboReset);
}

void ADefaultPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGameState->UpdatePlayerList();
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

void ADefaultPlayerState::HandleNoteDetected(ENoteResult InNoteResult)
{
	int32 ScoreToAdd = 0;
	bool isComboAdded = true;

	switch (InNoteResult)
	{
	case ENoteResult::Excellent:
		ScoreToAdd = 10;
		break;
	case ENoteResult::Good:
		ScoreToAdd = 5;
		break;
	case ENoteResult::Bad:
		ScoreToAdd = 0;
		isComboAdded = false;
		break;
	case ENoteResult::None:
		ScoreToAdd = -1;
		break;
	default:
		ScoreToAdd = -1;
		break;
	}

	if (ScoreToAdd == -1) return;

	// 클라이언트에서 서버로 점수 증가 요청
	if (!HasAuthority())
	{
		Server_AddScore(ScoreToAdd);
		isComboAdded ? Server_HandleCombo(false) : Server_HandleCombo(true);
	}
	else
	{
		// 서버에서 자기자신 점수 증가
		AddScore(ScoreToAdd);
		isComboAdded ? HandleCombo(false) : HandleCombo(true);
	}
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

void ADefaultPlayerState::OnRep_ComboState()
{
	OnComboChanged.Broadcast(this, CurrentCombo, bLastComboReset);
}

void ADefaultPlayerState::HandleCombo(bool isReset)
{
	if (isReset)
	{
		CurrentCombo = 0;
	}
	else
	{
		++CurrentCombo;
	}

	bLastComboReset = isReset;
}

void ADefaultPlayerState::Server_HandleCombo_Implementation(bool isReset)
{
	HandleCombo(isReset);
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		if (PC->IsLocalController())
		{
			OnComboChanged.Broadcast(this, CurrentCombo, bLastComboReset);
		}
	}
}
