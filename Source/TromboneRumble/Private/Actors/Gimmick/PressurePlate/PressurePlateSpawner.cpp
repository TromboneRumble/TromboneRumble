
#include "Actors/Gimmick/PressurePlate/PressurePlateSpawner.h"
#include "Actors/Gimmick/PressurePlate/PressurePlateBase.h"
#include "Engine/TargetPoint.h"
#include "Subsystems/RhythmSubsystem.h"

APressurePlateSpawner::APressurePlateSpawner()
{
	bReplicates = false;
}

void APressurePlateSpawner::Activate()
{
	Super::Activate();
	if (HasAuthority())
	{
		if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RS->OnMusicUserCue.AddDynamic(this, &ThisClass::OnMusicCueReceived);
		}
		TriggerPlateSpawn();
	}
}

void APressurePlateSpawner::Deactivate()
{
	Super::Deactivate();
	if (HasAuthority())
	{
		if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RS->OnMusicUserCue.RemoveDynamic(this, &ThisClass::OnMusicCueReceived);
		}
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void APressurePlateSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Deactivate();
	Super::EndPlay(EndPlayReason);
}

void APressurePlateSpawner::OnMusicCueReceived(FName CueName)
{
	if (CueName == TEXT("Event_Spotlight_Fever"))
	{
		bIsFeverTime = true;
		TriggerPlateSpawn();
	}
}

void APressurePlateSpawner::OnPlateDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority()) return;

	TObjectPtr<APressurePlateBase> DestroyedPlate = Cast<APressurePlateBase>(DestroyedActor);
	const TObjectPtr<ATargetPoint>* FoundKey = SpawnMap.FindKey(DestroyedPlate);

	if (FoundKey)
	{
		SpawnMap.Remove(*FoundKey);
	}

	if (!GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		const float Delay = bIsFeverTime ? RespawnDelay_Fever : RespawnDelay_Normal;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &APressurePlateSpawner::TriggerPlateSpawn, Delay, false);
	}
}

void APressurePlateSpawner::TriggerPlateSpawn()
{
	if (!HasAuthority()) return;

	const int32 MaxCount = bIsFeverTime ? MaxPlates_Fever : MaxPlates_Normal;

	if (SpawnMap.Num() < MaxCount)
	{
		//TriggerPoint중에 사용 가능한 Point반환
		TArray<TObjectPtr<ATargetPoint>> AvailablePoints;
		for (auto Point : SpawnPoints)
		{
			if (Point && !SpawnMap.Contains(Point))
			{
				AvailablePoints.Add(Point);
			}
		}

		if (AvailablePoints.Num() > 0)
		{
			int32 RandIndex = FMath::RandRange(0, AvailablePoints.Num() - 1);
			TObjectPtr<ATargetPoint> ChosenPoint = AvailablePoints[RandIndex];

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APressurePlateBase* NewPlate = GetWorld()->SpawnActor<APressurePlateBase>(
				PressurePlateClass,
				ChosenPoint->GetActorLocation(),
				ChosenPoint->GetActorRotation(),
				SpawnParams
			);

			if (NewPlate)
			{
				SpawnMap.Add(ChosenPoint, NewPlate);
				NewPlate->OnDestroyed.AddDynamic(this, &APressurePlateSpawner::OnPlateDestroyed);
			}
		}

		// 하나를 소환한 후에도 여전히 자리가 남아있다면 타이머를 다시 돌림
		if (SpawnMap.Num() < MaxCount)
		{
			const float Delay = bIsFeverTime ? RespawnDelay_Fever : RespawnDelay_Normal;
			GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &APressurePlateSpawner::TriggerPlateSpawn, Delay, false);
		}
	}
}