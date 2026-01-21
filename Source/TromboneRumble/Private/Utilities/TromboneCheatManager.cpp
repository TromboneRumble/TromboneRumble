// Fill out your copyright notice in the Description page of Project Settings.

#include "Utilities/TromboneCheatManager.h"
#include "Actors/Gimmick/Garbage/GarbageSpawner.h"
#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Utilities/EnumHelper.h"

void UTromboneCheatManager::Trombone_SpawnInstrument(const FString& TypeString)
{
	if (TypeString.IsEmpty())
	{
		const UEnum* EnumPtr = StaticEnum<EWeaponType>();
		FString AvailableTypes = TEXT("Available Types: ");
        
		for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
		{
			AvailableTypes += EnumPtr->GetNameStringByIndex(i);
			if (i < EnumPtr->NumEnums() - 2) AvailableTypes += TEXT(", ");
		}
        
		PRINT_WITH_CURRENT_CONTEXT(AvailableTypes);
		return;
	}
	
	EWeaponType Type;
	if (EnumHelper::StringToEnum<EWeaponType>(TypeString, Type))
	{
		APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
		if (!MyPawn) return;

		if (!WeaponClasses.Contains(Type))
		{
			PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Weapon class %s not assigned in CheatManager. talk to developer."), *TypeString));
			return;
		}

		FVector SpawnLocation = MyPawn->GetActorLocation() + FVector(0.f, 0.f, 200.f);
		FRotator SpawnRotation = MyPawn->GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MyPawn;
		SpawnParams.Instigator = MyPawn;

		AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(
			WeaponClasses[Type], 
			SpawnLocation, 
			SpawnRotation, 
			SpawnParams
		);

		if (SpawnedInstrument)
		{
			PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Spawned: %s"), *TypeString));
		}
	}
	else
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Invalid Instrument Type: %s"), *TypeString));
	}
}

void UTromboneCheatManager::Trombone_Spotlight()
{
	if (ASpotlightManager* Manager = Cast<ASpotlightManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASpotlightManager::StaticClass())))
	{
		Manager->Server_TriggerAllSpotlightSpawn();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Triggered All Spotlights"));
	}
}

void UTromboneCheatManager::Trombone_Throw(const FString& Count)
{
	const int32 CountInt = FCString::Atoi(*Count);

	if (CountInt >= 100)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("렉 걸려요. 100번 이상은 하지마세요 ㅎㅎ"));
		return;
	}
	
	if (AGarbageSpawner* Spawner = Cast<AGarbageSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AGarbageSpawner::StaticClass())))
	{
		Spawner->Server_SpawnGarbageForDebugging(CountInt);
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Garbage Spawned %s times"), *Count));
	}
}

void UTromboneCheatManager::Trombone_Ragdoll()
{
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(MyPawn))
	{
		TromboneCharacter->Server_DebugRagdoll();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Ragdoll executed"));
	}
}

void UTromboneCheatManager::Trombone_Stun()
{
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(MyPawn))
	{
		TromboneCharacter->Server_DebugStun();
		PRINT_WITH_CURRENT_CONTEXT(TEXT("Stun executed"));
	}
}
