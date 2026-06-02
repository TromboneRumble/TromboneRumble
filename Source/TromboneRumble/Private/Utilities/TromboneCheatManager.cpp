#include "Utilities/TromboneCheatManager.h"
#include "Actors/Gimmick/Garbage/GarbageSpawner.h"
#include "Actors/Gimmick/Spotlight/SpotlightManager.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/DefaultPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameStateSubsystem.h"
#include "Subsystems/SaveManagerSubsystem.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"
#include "Utilities/EnumHelper.h"

void UTromboneCheatManager::Trombone_Help()
{
	FString DebugMsg;
	DebugMsg += TEXT("사용 가능한 명령어:\n");
	DebugMsg += TEXT("Trombone_SpawnInstrument [InstrumentType] - 스폰할 악기 타입을 입력하여 악기를 소환합니다. (예: Trombone_SpawnInstrument Violin)\n");
	DebugMsg += TEXT("Trombone_Spotlight - 스포트라이트를 소환합니다.\n");
	DebugMsg += TEXT("Trombone_Throw [Count] - 쓰레기를 소환합니다. Count는 소환할 쓰레기의 수입니다. (예: Trombone_Throw 10)\n");
	DebugMsg += TEXT("Trombone_Ragdoll - 래그돌을 실행합니다.\n");
	DebugMsg += TEXT("Trombone_Stun - 스턴을 실행합니다.\n");
	DebugMsg += TEXT("Trombone_ResetSettingData - 설정 데이터 초기화\n");
	DebugMsg += TEXT("Trombone_SetCustomization [AntennaKey] [FaceKey] [CostumeKey] - 커스터마이징 즉시 변경 및 복제 (None=기본값, 예: Trombone_SetCustomization None Face_02 None)\n");
	DebugMsg += TEXT("--------------------------------\n");
	DebugMsg += TEXT("스폰 가능한 악기 타입 목록 :\n");
	DebugMsg += TEXT("Trombone, Violin, Cymbal\n");
	DebugMsg += TEXT("--------------------------------\n");
	DebugMsg += TEXT("대소문자는 상관없습니다.\n");

	PRINT_WITH_CURRENT_CONTEXT(DebugMsg);
}

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
	if (!EnumHelper::StringToEnum<EWeaponType>(TypeString, Type))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("악기 타입이 잘못되었습니다: %s"), *TypeString));
		PRINT_WITH_CURRENT_CONTEXT(TEXT("가능한 악기 타입: Trombone, Violin, Cymbals"));
		return;
	}
	
	APawn* MyPawn = GetOuterAPlayerController()->GetPawn();
	if (!MyPawn) return;
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
	if (Config->InstrumentClasses.Num() == 0)
	{
		PRINT_WITH_CURRENT_CONTEXT(TEXT("No weapon classes assigned in Project Settings. Assign weapon classes"));
		return;
	}

	if (!Config->InstrumentClasses.Contains(Type))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Weapon class %s not assigned in CheatManager. talk to developer."), *TypeString));
		return;
	}

	FVector SpawnLocation = MyPawn->GetActorLocation() + FVector(0.f, 0.f, 200.f);
	FRotator SpawnRotation = MyPawn->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyPawn;
	SpawnParams.Instigator = MyPawn;

	if (GetWorld()->SpawnActor<AActor>(Config->InstrumentClasses[Type].LoadSynchronous(), SpawnLocation, SpawnRotation, SpawnParams))
	{
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("%s 스폰"), *TypeString));
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

void UTromboneCheatManager::Trombone_ResetSettingData()
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (USaveManagerSubsystem* Subsystem = GI->GetSubsystem<USaveManagerSubsystem>())
			{
				Subsystem->ResetToDefaultSettings();
				PRINT_WITH_CURRENT_CONTEXT(TEXT("Tutorial data reset"));
			}
		}
	}
}

void UTromboneCheatManager::Trombone_Dump_LevelStateSubsystem()
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (const UGameStateSubsystem* Subsystem = GI->GetSubsystem<UGameStateSubsystem>())
			{
				Subsystem->DumpSettings();
			}
		}
	}
}
