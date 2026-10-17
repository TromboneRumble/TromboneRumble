#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Data/Gimmick/SpotlightGimmickConfig.h"
#include "Actors/Gimmick/Spotlight/SpotlightZone.h"
#include "Data/QuestData.h"
#include "Engine/TargetPoint.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"

ASpotlightManager::ASpotlightManager()
{
	SetReplicates(false);
}

const USpawnerGimmickConfig& ASpotlightManager::GetSpawnerConfig() const
{
	return GetConfig<USpotlightGimmickConfig>();
}

void ASpotlightManager::SpawnBatch(const int32 SpawnCount)
{
	if (!HasAuthority()) return;

    const USpotlightGimmickConfig& Config = GetConfig<USpotlightGimmickConfig>();
    const bool bIsFeverTime = IsFeverTime();
    
    TArray<TObjectPtr<ATargetPoint>> AvailableSpawnPoints;
    for (auto Point : SpawnPoints)
    {
        if (Point && !IsSpawnPointOccupied(Point->GetActorLocation()))
        {
            AvailableSpawnPoints.Add(Point);
        }
    }
    
    const int32 ActualSpawnCount = FMath::Min(SpawnCount, AvailableSpawnPoints.Num());
    
    for (int32 i = 0; i < ActualSpawnCount; ++i)
    {
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
                NewZone->InitializeZone(bIsFeverTime, Config.BonusScore);
                ActiveSpotlightZones.Add(NewZone);
                NewZone->OnDestroyed.AddDynamic(this, &ASpotlightManager::OnSpotlightZoneDestroyed);

                if (UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
                {
                    NewZone->OnSpotlightBonusEarned.AddUObject(TutorialSub, 
                        &UTutorialWorldSubsystem::ReportAction, 
                        EQuestConditionType::HitSpotlight, 
                        EQuestConditionParamType::Any,
                        FString());
                }
            }
        }
    }
}

bool ASpotlightManager::IsSpawnPointOccupied(const FVector& Location) const
{
    for (const auto& Zone : ActiveSpotlightZones)
    {
        if (Zone && FVector::DistSquared(Zone->GetActorLocation(), Location) < 100.f)
        {
            return true;
        }
    }
    return false;
}

void ASpotlightManager::OnSpotlightZoneDestroyed(AActor* DestroyedActor)
{
    if (ASpotlightZone* Zone = Cast<ASpotlightZone>(DestroyedActor))
    {
        ActiveSpotlightZones.Remove(Zone);
        if (const UTutorialWorldSubsystem* TutorialSub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
        {
            Zone->OnSpotlightBonusEarned.RemoveAll(TutorialSub);
        }
    }
}

void ASpotlightManager::Server_TriggerAllSpotlightSpawn_Implementation()
{
    if (!HasAuthority()) return;

    for (TObjectPtr Point : SpawnPoints)
    {
        if (!Point) continue;

        if (IsSpawnPointOccupied(Point->GetActorLocation())) continue;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ASpotlightZone* NewZone = GetWorld()->SpawnActor<ASpotlightZone>(
            SpotlightZoneClass,
            Point->GetActorLocation(),
            Point->GetActorRotation(),
            SpawnParams
        );

        if (NewZone)
        {
            NewZone->InitializeZone(IsFeverTime(), GetConfig<USpotlightGimmickConfig>().BonusScore);
            ActiveSpotlightZones.Add(NewZone);
            NewZone->OnDestroyed.AddDynamic(this, &ASpotlightManager::OnSpotlightZoneDestroyed);
        }
    }
}
