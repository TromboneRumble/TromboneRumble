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
	DOREPLIFETIME(ThisClass, ComboData);
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
		break;
	case ENoteResult::None:
		return;
	default:
		return; 
	}

	//콤보 UI 반영은 클라이언트에서 즉시 반영
	HandleCombo(InNoteResult);

	// 클라이언트에서 서버로 점수 증가 요청
	if (HasAuthority())
	{
		AddScore(ScoreToAdd);
	}
	else
	{
		Server_HandleCombo(InNoteResult);
		Server_AddScore(ScoreToAdd);
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

void ADefaultPlayerState::OnRep_ComboData()
{
	// 만약 이 PlayerState가 Local Player라면 HandleNoteDetected()에서 이미 UI를 띄웠음.
	// 그러므로 Return
	if (APlayerController* PC = Cast<APlayerController>(GetOwningController()))
	{
		if (PC->IsLocalController())
		{
			return;
		}
	}

	//다른 플레이어의 콤보가 바뀌었을때 UI 처리용
	OnComboChanged.Broadcast(ComboData.LastNoteResult, ComboData.CurrentCombo);
}

void ADefaultPlayerState::HandleCombo(ENoteResult InResult)
{
	FComboData NewData = ComboData;

	if (InResult == ENoteResult::Bad || InResult == ENoteResult::None)
	{
		NewData.CurrentCombo = 0;
	}
	else
	{
		NewData.CurrentCombo++;
	}

	NewData.LastNoteResult = InResult;

	// Replication 트리거용은 서버에서만 관리
	// 클라에선 단순히 개인 UI 갱신용으로 값을 바꾸지 않음.
	if (HasAuthority())
	{
		NewData.TransactionID++;
	}

	if (!(ComboData == NewData))
	{
		ComboData = NewData;
		OnComboChanged.Broadcast(ComboData.LastNoteResult, ComboData.CurrentCombo);
	}
}

void ADefaultPlayerState::Server_HandleCombo_Implementation(ENoteResult InResult)
{
	HandleCombo(InResult);
}