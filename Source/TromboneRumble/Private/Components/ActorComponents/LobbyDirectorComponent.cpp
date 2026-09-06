// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/LobbyDirectorComponent.h"
#include "EasySessionStatics.h"
#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Data/RhythmSongDataRow.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/LobbyGameState.h"
#include "Framework/TromboneGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Items/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameDataSubsystem.h"
#include "Utilities/DebugHelper.h"

ULobbyDirectorComponent::ULobbyDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULobbyDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AbortLobbyFlow();

	Super::EndPlay(EndPlayReason);
}

void ULobbyDirectorComponent::StartLobbyFlow()
{
	OwnerGameMode = Cast<AGameModeBase>(GetOwner());
	if (!OwnerGameMode)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("LobbyDirectorComponent must be attached to a GameMode!"));
		return;
	}

	LobbyGameState = OwnerGameMode->GetGameState<ALobbyGameState>();
	if (!LobbyGameState)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("LobbyGameState not found!"));
		return;
	}

	CollectFallingSpawnPoints();
	
	SetLobbyState(ELobbyState::WaitingForPlayers);
}

void ULobbyDirectorComponent::AbortLobbyFlow() const
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void ULobbyDirectorComponent::HandlePlayerReady(APlayerController* ReadyPlayer)
{
	if (!ReadyPlayer)
	{
		return;
	}

	// 늦게 합류했다면 즉시 낙하시키고 스스로 기상
	if (!IsInLobbyState(ELobbyState::WaitingForPlayers))
	{
		LaunchPlayerFalling(ReadyPlayer, 0);
		return;
	}

	WaitingPlayers.AddUnique(ReadyPlayer);
	
	PrepareWaitingPlayer(ReadyPlayer, 0);
}

void ULobbyDirectorComponent::HandleAllPlayersReady()
{
	if (!LobbyGameState || !IsInLobbyState(ELobbyState::WaitingForPlayers))
	{
		return;
	}

	const FGameplayTag InGameMapTag = UEasyStatics::GetCurrentInGameMap(this, UTromboneConfig::Get()->DefaultInGameMap);
	const FGameplayTag SelectedSong = UTromboneFunctionLibrary::PickRandomSongForMap(InGameMapTag);
	LobbyGameState->SetSelectedSongTag(SelectedSong);

	GetWorld()->GetTimerManager().ClearTimer(PreFallTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(PreFallTimerHandle, this, &ThisClass::OnPreFallTimerFinished, FallStartDelay, false);
}

void ULobbyDirectorComponent::NotifyItemEquipped(APawn* EquippedPlayer, AItemBase* EquippedItem)
{
	if (!IsValid(EquippedPlayer) || !EquippedItem)
	{
		return;
	}

	if (!IsInLobbyState(ELobbyState::InstrumentScramble))
	{
		return;
	}

	if (const AWeaponBase* Weapon = Cast<AWeaponBase>(EquippedItem))
	{
		if (Weapon->GetWeaponType() == EWeaponType::Headbutt)
		{
			return;
		}
	}

	if (++EquippedInstrumentCount >= SpawnedInstrumentCount)
	{
		SetLobbyState(ELobbyState::CountdownToTravel);
	}
}

void ULobbyDirectorComponent::SetLobbyState(const ELobbyState& InNewState)
{
	if (!LobbyGameState)
	{
		LOG_WITH_CURRENT_CONTEXT(Error, TEXT("LobbyGameState is null!"));
		return;
	}

	const ELobbyState CurrentState = LobbyGameState->GetCurrentLobbyState();
	if (CurrentState == InNewState)
	{
		return;
	}

	LobbyGameState->SetLobbyState(InNewState);

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	switch (InNewState)
	{
		case ELobbyState::WaitingForPlayers:
			break;

		case ELobbyState::FallingPlayers:
			LaunchAllPlayersFalling();
			StartGroundedPolling();
			break;

		case ELobbyState::CountdownToStandup:
			{
				SpawnInstruments();

				const int32 StandupSeconds = UTromboneConfig::Get() ? UTromboneConfig::Get()->LobbyRagdollGetUpDelaySeconds : 5;
				TimerManager.ClearTimer(StandupTimerHandle);
				TimerManager.SetTimer(StandupTimerHandle, this, &ThisClass::OnStandupCountdownFinished, static_cast<float>(StandupSeconds), false);
			}
			break;

		case ELobbyState::InstrumentScramble:
			break;

		case ELobbyState::CountdownToTravel:
			{
				const int32 TravelSeconds = UTromboneConfig::Get() ? UTromboneConfig::Get()->LobbyGameStartDelaySeconds : 5;
				TimerManager.ClearTimer(TravelTimerHandle);
				TimerManager.SetTimer(TravelTimerHandle, this, &ThisClass::OnTravelCountdownTimerFinished, static_cast<float>(TravelSeconds), false);
			}
			break;

		default:
			break;
	}
}

void ULobbyDirectorComponent::PrepareWaitingPlayer(APlayerController* PC, const int32 RetryCount)
{
	if (!IsValid(PC))
	{
		return;
	}

	ATromboneCharacterBase* Character = Cast<ATromboneCharacterBase>(PC->GetPawn());
	if (!Character)
	{
		// Seamless Travel/RestartPlayer 타이밍에 따라 폰이 아직 없을 수 있으므로 유한 재시도
		if (RetryCount < MaxPawnRetryCount)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
				[this, WeakPC = TWeakObjectPtr(PC), RetryCount]()
				{
					if (WeakPC.IsValid()) { PrepareWaitingPlayer(WeakPC.Get(), RetryCount + 1); }
				}));
		}
		else
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to prepare %s for falling: no pawn after %d retries"), *PC->GetName(), MaxPawnRetryCount));
		}
		return;
	}

	Character->GetCharacterMovement()->DisableMovement();
	Character->SetActorHiddenInGame(true);
	Character->Server_SetInputEnabled(false);
}

void ULobbyDirectorComponent::OnPreFallTimerFinished()
{
	SetLobbyState(ELobbyState::FallingPlayers);
}

void ULobbyDirectorComponent::CollectFallingSpawnPoints()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("FallingSpawnPoint"), FoundActors);

	FoundActors.Sort([](const AActor& A, const AActor& B) { return A.GetName() < B.GetName(); });

	FallingSpawnPoints.Empty(FoundActors.Num());
	for (AActor* FoundActor : FoundActors)
	{
		FallingSpawnPoints.Add(FoundActor);
	}

	if (FallingSpawnPoints.Num() == 0)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No FallingSpawnPoint actors found in the lobby map. Falling back to spawn location + FallHeight."));
	}
}

void ULobbyDirectorComponent::LaunchAllPlayersFalling()
{
	for (const TObjectPtr<APlayerController>& PC : WaitingPlayers)
	{
		LaunchPlayerFalling(PC, 0);
	}
	WaitingPlayers.Empty();
}

void ULobbyDirectorComponent::RelaunchAllPlayersFalling()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		LaunchPlayerFalling(It->Get(), 0);
	}
}

void ULobbyDirectorComponent::LaunchPlayerFalling(APlayerController* PC, const int32 RetryCount)
{
	if (!IsValid(PC))
	{
		return;
	}

	ATromboneCharacterBase* Character = Cast<ATromboneCharacterBase>(PC->GetPawn());
	if (!Character)
	{
		if (RetryCount < MaxPawnRetryCount)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
				[this, WeakPC = TWeakObjectPtr(PC), RetryCount]()
				{
					if (WeakPC.IsValid()) { LaunchPlayerFalling(WeakPC.Get(), RetryCount + 1); }
				}));
		}
		else
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to start falling for %s: no pawn after %d retries"), *PC->GetName(), MaxPawnRetryCount));
		}
		return;
	}

	UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent();
	if (!Ragdoll || Ragdoll->IsRagdoll())
	{
		return;
	}

	Character->SetActorHiddenInGame(false);
	Character->Server_SetInputEnabled(true);

	// 떨어지고 있거나 일어나기 전 카운트 다운 중에만 자동 일어나기 안되게
	const bool bAutoGetUp = !IsInLobbyState(ELobbyState::FallingPlayers) && !IsInLobbyState(ELobbyState::CountdownToStandup);

	FVector DropLocation;
	const AActor* SpawnPoint = FallingSpawnPoints.Num() > 0
		? FallingSpawnPoints[NextFallingSpawnPointIndex++ % FallingSpawnPoints.Num()].Get()
		: nullptr;
	if (IsValid(SpawnPoint))
	{
		DropLocation = SpawnPoint->GetActorLocation();
	}
	else
	{
		// fallback
		DropLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, FallHeight);
	}

	const FRotator DropRotation(
		FMath::FRandRange(-FallAngle, FallAngle),
		FMath::FRandRange(0.0f, 360.0f),
		FMath::FRandRange(-FallAngle, FallAngle));

	Character->SetActorLocationAndRotation(DropLocation, DropRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector TumbleAxis = FMath::VRand().GetSafeNormal2D();
	const FVector InitialAngularVelocity = TumbleAxis * FMath::FRandRange(0.0f, FallRotationRate);

	Ragdoll->SetAutoGetUpEnabled(bAutoGetUp);
	Ragdoll->StartRagdoll(FVector::ZeroVector, InitialAngularVelocity);

	Character->Multicast_PlayFallScream();
	Character->SetLandingSoundEnabled(true);
}

void ULobbyDirectorComponent::StartGroundedPolling()
{
	GroundPollElapsed = 0.0f;
	GetWorld()->GetTimerManager().ClearTimer(GroundPollTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(GroundPollTimerHandle, this, &ThisClass::PollAllGrounded, GroundPollInterval, true);
}

void ULobbyDirectorComponent::PollAllGrounded()
{
	GroundPollElapsed += GroundPollInterval;
	const bool bForce = GroundPollElapsed >= MaxGetUpWaitTime;

	// 모든 플레이어가 땅에 떨어지기를 기다리기
	bool bAllGrounded = true;
	bool bAnyRagdolling = false;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const ATromboneCharacterBase* Character = PC ? Cast<ATromboneCharacterBase>(PC->GetPawn()) : nullptr;
		const UTromboneRagdollComponent* Ragdoll = Character ? Character->GetRagdollComponent() : nullptr;
		if (!Ragdoll || !Ragdoll->IsRagdoll())
		{
			continue;
		}

		bAnyRagdolling = true;
		if (!Ragdoll->IsRagdollResting())
		{
			bAllGrounded = false;
		}
	}

	if ((!bAnyRagdolling || !bAllGrounded) && !bForce)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(GroundPollTimerHandle);
	SetLobbyState(ELobbyState::CountdownToStandup);
}

void ULobbyDirectorComponent::OnStandupCountdownFinished()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const ATromboneCharacterBase* Character = PC ? Cast<ATromboneCharacterBase>(PC->GetPawn()) : nullptr;
		UTromboneRagdollComponent* Ragdoll = Character ? Character->GetRagdollComponent() : nullptr;
		if (!Ragdoll || !Ragdoll->IsRagdoll())
		{
			continue;
		}

		Ragdoll->SetAutoGetUpEnabled(true);
		Ragdoll->StopRagdoll();
	}

	SetLobbyState(ELobbyState::InstrumentScramble);
}

void ULobbyDirectorComponent::SpawnInstruments()
{
	const UTromboneGameInstance* GameInstance = OwnerGameMode ? Cast<UTromboneGameInstance>(OwnerGameMode->GetGameInstance()) : nullptr;
	if (!GameInstance)
	{
		return;
	}

	const auto* DataSubsystem = GameInstance->GetSubsystem<UGameDataSubsystem>();
	const FGameplayTag SongTag = GameInstance->GetSelectedSongTag();
	const FRhythmSongDataRow* SongRow = DataSubsystem->GetSongRow(SongTag);
	if (!SongRow)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("SongRow not found"));
		return;
	}

	const auto& InstrumentSounds = SongRow->InstrumentSounds;
	if (InstrumentSounds.Num() == 0)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No instrument sounds found for the selected song"));
		return;
	}

	TArray<AActor*> SpawnPointActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("InstrumentSpawnPoint"), SpawnPointActors);

	if (SpawnPointActors.Num() == 0)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("No spawn points found for instruments"));
		return;
	}

	SpawnedInstrumentCount = FMath::Clamp(OwnerGameMode->GetNumPlayers() - 1, 1, 3);

	for (int32 i = 0; i < SpawnedInstrumentCount; i++)
	{
		const int32 SpawnPointIndex = i % SpawnPointActors.Num();
		const AActor* SpawnPoint = SpawnPointActors[SpawnPointIndex];
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		const FRotator SpawnRotation = SpawnPoint->GetActorRotation();

		const int32 InstrumentClassIndex = i % InstrumentSounds.Num();
		TSubclassOf<AWeaponBase> ClassToSpawn = InstrumentSounds[InstrumentClassIndex].SpawnInstrument;

		GetWorld()->SpawnActor<AWeaponBase>(ClassToSpawn, SpawnLocation, SpawnRotation);
	}
}

void ULobbyDirectorComponent::OnTravelCountdownTimerFinished()
{
	OnTravelCountdownFinished.Broadcast();
}

bool ULobbyDirectorComponent::IsInLobbyState(const ELobbyState State) const
{
	return LobbyGameState && LobbyGameState->IsInState(State);
}
