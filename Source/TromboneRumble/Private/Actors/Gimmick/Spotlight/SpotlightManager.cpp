#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Actors/Gimmick/Spotlight/SpotlightZone.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/RhythmSubsystem.h"

ASpotlightManager::ASpotlightManager()
{
	SetReplicates(false);
}

void ASpotlightManager::Activate()
{
    Super::Activate();
    
    if (HasAuthority())
    {
        if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
        {
            RS->OnMusicCallback.AddDynamic(this, &ThisClass::OnMusicCallbackReceived);
        }
        TriggerSpotlightSpawn();
    }
}

void ASpotlightManager::Deactivate()
{
    Super::Deactivate();
    
    if (HasAuthority())
    {
        if (GetWorld())
        {
            GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
            SpawnTimerHandle.Invalidate();
        }

        if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
        {
            RS->OnMusicCallback.RemoveDynamic(this, &ThisClass::OnMusicCallbackReceived);
        }
    }
}

void ASpotlightManager::BeginPlay()
{
    Super::BeginPlay();
    
    TutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
}

void ASpotlightManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Deactivate();
	
	Super::EndPlay(EndPlayReason);
}

void ASpotlightManager::OnMusicCallbackReceived(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
    if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
    {
        const FString& CueString = MusicInfo->UserCueName;
        if (!CueString.IsEmpty())
        {
            const FName CueName(*CueString);
            if (CueName == TEXT("Event_Spotlight_Fever"))
            {
                bIsFeverTime = true;
            }
        }
    }
}

void ASpotlightManager::TriggerSpotlightSpawn()
{
	if (!HasAuthority()) return;

    const int32 MinCount = bIsFeverTime ? MinSpawnCount_Fever : MinSpawnCount_Normal;
    const int32 MaxCount = bIsFeverTime ? MaxSpawnCount_Fever : MaxSpawnCount_Normal;
    const float MinInterval = bIsFeverTime ? MinSpawnInterval_Fever : MinSpawnInterval_Normal;
    const float MaxInterval = bIsFeverTime ? MaxSpawnInterval_Fever : MaxSpawnInterval_Fever;

    const int32 SpawnCount = FMath::RandRange(MinCount, MaxCount);
    
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
                NewZone->InitializeZone(bIsFeverTime, SpotlightBonusScore);
                ActiveSpotlightZones.Add(NewZone);
                NewZone->OnDestroyed.AddDynamic(this, &ASpotlightManager::OnSpotlightZoneDestroyed);
                
                if (TutorialManager)
                {
                    NewZone->OnSpotlightBonusEarned.AddUObject(TutorialManager, &ATutorialManager::HandleOnSpotlightBonusEarned);
                }
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
        if (TutorialManager)
        {
            Zone->OnSpotlightBonusEarned.RemoveAll(TutorialManager);
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
            NewZone->InitializeZone(bIsFeverTime, SpotlightBonusScore);
            ActiveSpotlightZones.Add(NewZone);
            NewZone->OnDestroyed.AddDynamic(this, &ASpotlightManager::OnSpotlightZoneDestroyed);
        }
    }
}
