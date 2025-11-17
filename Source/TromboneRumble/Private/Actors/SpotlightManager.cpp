// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/SpotlightManager.h"
#include "Actors/SpotlightZone.h"
#include "Engine/TargetPoint.h"
#include "Framework/InGameState.h"
#include "Utilities/DebugHelper.h"

ASpotlightManager::ASpotlightManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(false);
}

void ASpotlightManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CachedInGameState = GetWorld()->GetGameState<AInGameState>();
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void ASpotlightManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	Super::EndPlay(EndPlayReason);
}

void ASpotlightManager::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsSpotlightActive)
	{
		CheckSpotlightStart();
	}
}

void ASpotlightManager::CheckSpotlightStart()
{
	if (!CachedInGameState)
	{
		CachedInGameState = GetWorld()->GetGameState<AInGameState>();
		if (!CachedInGameState) return;
	}

	if (GetCurrentSongProgress() >= SpotlightStartTimePercent)
	{
		bIsSpotlightActive = true;
		SetActorTickEnabled(false);
		TriggerSpotlightSpawn();
	}
}

void ASpotlightManager::TriggerSpotlightSpawn()
{
	if (!HasAuthority()) return;

    const float CurrentProgress = GetCurrentSongProgress();
    const bool bIsFeverTime = (CurrentProgress >= FeverTimeStartPercent);

    const int32 MinCount = bIsFeverTime ? MinSpawnCount_Fever : MinSpawnCount_Normal;
    const int32 MaxCount = bIsFeverTime ? MaxSpawnCount_Fever : MaxSpawnCount_Normal;
    const float MinInterval = bIsFeverTime ? MinSpawnInterval_Fever : MinSpawnInterval_Normal;
    const float MaxInterval = bIsFeverTime ? MaxSpawnInterval_Fever : MaxSpawnInterval_Fever;

    const int32 SpawnCount = FMath::RandRange(MinCount, MaxCount);
    
    TArray<TObjectPtr<ATargetPoint>> AvailableSpawnPoints = SpawnPoints;

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        if (AvailableSpawnPoints.Num() == 0) break; 

        const int32 RandIndex = FMath::RandRange(0, AvailableSpawnPoints.Num() - 1);
        TObjectPtr<ATargetPoint> ChosenPoint = AvailableSpawnPoints[RandIndex];

        AvailableSpawnPoints.RemoveAt(RandIndex);

        if (ChosenPoint)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            ASpotlightZone* NewZone = GetWorld()->SpawnActor<ASpotlightZone>(
                SpotlightZoneClass,
                ChosenPoint->GetActorLocation(),
                ChosenPoint->GetActorRotation(),
                SpawnParams
            );

            if (NewZone)
            {
                NewZone->InitializeZone(bIsFeverTime);
            }
        }
    }

    const float NextSpawnInterval = FMath::RandRange(MinInterval, MaxInterval);

    GetWorldTimerManager().ClearTimer(SpawnTimerHandle); 
    
    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &ASpotlightManager::TriggerSpotlightSpawn,
        NextSpawnInterval,
        false
    );
}

float ASpotlightManager::GetCurrentSongProgress() const
{
	if (CachedInGameState)
	{
		return CachedInGameState->GetSongProgress();
	}
	return 0.0f;
}

