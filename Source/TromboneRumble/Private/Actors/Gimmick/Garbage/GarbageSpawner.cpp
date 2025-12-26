// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/Garbage/GarbageSpawner.h"
#include "Actors/Gimmick/Garbage/GarbageBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Framework/InGameState.h"
#include "GameFramework/PlayerState.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"

AGarbageSpawner::AGarbageSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(false);
}


void AGarbageSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && bAutoStart)
	{
		if (bAutoStart)
		{
			StartAutoSpawn_Server();
		}
		else
		{
			if (URhythmSubsystem* MusicCueSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
			{
				MusicCueSubsystem->OnMusicUserCue.AddDynamic(this, &ThisClass::StartAutoSpawnFromMusicCue);
			}
		}
	}	
}

void AGarbageSpawner::SpawnGarbageOnce_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	if (GarbageClasses.Num() == 0 || SpawnPointActors.Num() == 0)
	{
		return;
	}
	
	AActor* TargetPawn = PickTargetPawn();
	if (!IsValid(TargetPawn))
	{
		return;
	}

	FTransform SpawnTransform;
	if (!PickRandomSpawnTransform(SpawnTransform))
	{
		return;
	}

	SpawnAndThrow_Server(SpawnTransform, TargetPawn);
}

void AGarbageSpawner::StartAutoSpawn_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AutoSpawnTimer.IsValid())
	{
		return;
	}
	
	const float SpawnInterval = UKismetMathLibrary::RandomFloatInRange(SpawnIntervalMin, SpawnIntervalMax);

	GetWorldTimerManager().SetTimer(
		AutoSpawnTimer,
		this,
		&ThisClass::SpawnGarbageOnce_Server,
		SpawnInterval,
		true
	);
}

void AGarbageSpawner::StopAutoSpawn_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AutoSpawnTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(AutoSpawnTimer);
	}
}

void AGarbageSpawner::StartAutoSpawnFromMusicCue(FName CueName)
{
	if (CueName == TEXT("Event_Spotlight_Start"))
	{
		StartAutoSpawn_Server();
	}
}

TSubclassOf<AGarbageBase> AGarbageSpawner::PickRandomGarbageClass() const
{
	const int32 Index = FMath::RandRange(0, GarbageClasses.Num() - 1);
	return GarbageClasses[Index];
}

bool AGarbageSpawner::PickRandomSpawnTransform(FTransform& OutTransform) const
{
	TArray<AActor*> ValidPoints;

	for (AActor* Point : SpawnPointActors)
	{
		if (IsValid(Point))
		{
			ValidPoints.Add(Point);
		}
	}

	if (ValidPoints.Num() == 0)
	{
		return false;
	}

	const int32 Index = FMath::RandRange(0, ValidPoints.Num() - 1);
	OutTransform = ValidPoints[Index]->GetActorTransform();
	return true;
}

AActor* AGarbageSpawner::PickTargetPawn() const
{
	switch (TargetingMode)
	{
	case EGarbageTargetingMode::Random:
		return PickRandomPlayerPawn();

	case EGarbageTargetingMode::ScoreWeighted:
		return PickScoreWeightedTargetPawn();

	default:
		return PickRandomPlayerPawn();
	}
}

AActor* AGarbageSpawner::PickRandomPlayerPawn() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> Candidates;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC))
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (!IsValid(Pawn))
		{
			continue;
		}

		Candidates.Add(Pawn);
	}

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
	return Candidates[Index];
}

AActor* AGarbageSpawner::PickScoreWeightedTargetPawn() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AInGameState* InGameState = World->GetGameState<AInGameState>();
	if (!InGameState)
	{
		return nullptr;
	}

	TArray<APlayerState*> SortedPlayers;
	InGameState->GetPlayersSortedByScore(SortedPlayers);

	const int32 NumPlayers = SortedPlayers.Num();
	if (NumPlayers == 0)
	{
		return nullptr;
	}

	// 가중치 합: 1 + 2 + ... + N = N(N+1)/2
	const int32 TotalWeightInt = NumPlayers * (NumPlayers + 1) / 2;
	const float TotalWeight = static_cast<float>(TotalWeightInt);
	const float RandomPoint = FMath::FRand() * TotalWeight;

	float Accumulated = 0.0f;
	for (int32 Index = 0; Index < NumPlayers; ++Index)
	{
		APlayerState* PS = SortedPlayers[Index];
		if (!IsValid(PS))
		{
			continue;
		}

		// Index 0 → 1등 → weight = NumPlayers
		// Index 1 → 2등 → weight = NumPlayers - 1
		const int32 WeightInt = NumPlayers - Index;
		const float Weight = static_cast<float>(WeightInt);

		Accumulated += Weight;

		if (RandomPoint <= Accumulated)
		{
			APawn* SelectedPawn = PS->GetPawn();
			return SelectedPawn;
		}
	}

	return PickRandomPlayerPawn();
}

void AGarbageSpawner::SpawnAndThrow_Server(const FTransform& SpawnTransform, AActor* TargetPawn)
{
	if (!HasAuthority())
	{
		return;
	}

	const TSubclassOf<AGarbageBase> GarbageClass = PickRandomGarbageClass();
	if (!GarbageClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AGarbageBase* Garbage = World->SpawnActor<AGarbageBase>(GarbageClass, SpawnTransform, Params);
	if (!IsValid(Garbage))
	{
		return;
	}

	const FVector StartLocation = SpawnTransform.GetLocation();

	const FVector TargetBase = TargetPawn->GetActorLocation();
	const FVector TargetOffset(
		FMath::FRandRange(-TargetRandomRadius, TargetRandomRadius),
		FMath::FRandRange(-TargetRandomRadius, TargetRandomRadius),
		0.f
	);

	const FVector TargetLocation = TargetBase + TargetOffset;

	Garbage->InitThrow_Server(StartLocation, TargetLocation);
}
