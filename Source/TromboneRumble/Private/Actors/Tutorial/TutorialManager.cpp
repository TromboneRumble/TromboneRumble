#include "Actors/Tutorial/TutorialManager.h"
#include "AkGameplayStatics.h"
#include "Actors/Tutorial/TutorialDummy.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "UI/UserWidgets/Popup/TwoButtonPopup.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneStatics.h"

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (UTutorialWorldSubsystem* Sub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		Sub->RegisterManager(this);
		Sub->OnDialogueSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialDialogueSequence);
		Sub->OnQuestSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialQuestSequence);
		Sub->OnTransitionSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialTransitionSequence);
	}
	
	ToggleTutorialBGM(true);
}

void ATutorialManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTutorialWorldSubsystem* Sub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		Sub->OnDialogueSequenceEvent.RemoveAll(this);
		Sub->OnQuestSequenceEvent.RemoveAll(this);
		Sub->OnTransitionSequenceEvent.RemoveAll(this);
		Sub->UnregisterManager();
	}
	
	Super::EndPlay(EndPlayReason);
}

void ATutorialManager::OnTutorialDialogueSequence(const FText& DialogueString)
{
	TogglePlayerInput(false);
}

void ATutorialManager::OnTutorialQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray)
{
	TogglePlayerInput(true);
}

void ATutorialManager::OnTutorialTransitionSequence()
{
	TogglePlayerInput(false);
}

void ATutorialManager::TogglePlayerInput(bool bIsEnabled)
{
	if (ADefaultTromboneCharacter* MyCharacter = GetCachedPlayerCharacter())
	{
		MyCharacter->SetPlayerInput(bIsEnabled);
	}
}

void ATutorialManager::ToggleTutorialBGM(bool bIsOn)
{
	if (bIsOn)
	{
		if (TutorialBGMEvent)
		{
			UAkGameplayStatics::PostEvent(TutorialBGMEvent, UGameplayStatics::GetPlayerPawn(this,0), 0, FOnAkPostEventCallback());
		}
	}
	else
	{
		if (TutorialBGMOffSwitch)
		{
			UAkGameplayStatics::SetSwitch(TutorialBGMOffSwitch, UGameplayStatics::GetPlayerPawn(this, 0),FName(""),FName(""));
		}
	}
}

void ATutorialManager::UnequipMyCharacter()
{
	if (ADefaultTromboneCharacter* MyCharacter = GetCachedPlayerCharacter())
	{
		MyCharacter->Unequip();
	}
}

void ATutorialManager::SpawnInstruments()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
	
	for (const auto Elem : Config->InstrumentClasses)
	{
		const EWeaponType WeaponType = Elem.Key;
		const TSoftClassPtr<AInstrumentBase>& SoftClassPtr = Elem.Value;

		UClass* LoadedClass = SoftClassPtr.LoadSynchronous();
		if (!LoadedClass)
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to load class: %s"), *EnumHelper::EnumToString(WeaponType)));
			continue;
		}

		FVector SpawnLocation = WeaponSpawnLocations.Contains(WeaponType) ? WeaponSpawnLocations[WeaponType] : FVector::ZeroVector;

		if (AInstrumentBase* SpawnedInstrument = GetWorld()->SpawnActor<AInstrumentBase>(LoadedClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		{
			SpawnedInstrument->SetCanBeSwitched(true);
			SpawnedInstruments.Add(SpawnedInstrument);
		}
		else
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to spawn instrument: %s"), *LoadedClass->GetName()));
		}
	}
}

AInstrumentBase* ATutorialManager::SpawnInstrument(EWeaponType WeaponType)
{
	if (WeaponType == EWeaponType::Invalid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid weapon type provided for spawning instrument"));
		return nullptr;
	}
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
	
	if (!Config->InstrumentClasses.Contains(WeaponType))
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon class found for weapon type %s"), *EnumHelper::EnumToString(WeaponType));
		return nullptr;
	}

	const TSubclassOf<AActor> WeaponClass = Config->InstrumentClasses[WeaponType].LoadSynchronous();
	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon class for weapon type %s is null"), *EnumHelper::EnumToString(WeaponType));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLocation = WeaponSpawnLocations.Contains(WeaponType) ? WeaponSpawnLocations[WeaponType] : FVector::ZeroVector;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	if (AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(WeaponClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		SpawnedInstruments.Add(SpawnedInstrument);
		return Cast<AInstrumentBase>(SpawnedInstrument);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Failed to spawn instrument of class %s"), *WeaponClass->GetName());
	return nullptr;
}

void ATutorialManager::DestroySpawnedInstruments()
{
	for (int32 i = SpawnedInstruments.Num() - 1; i >= 0; --i)
	{
		AActor* Instrument = SpawnedInstruments[i].Get();
		if (IsValid(Instrument))
		{
			Instrument->Destroy();
		}
	}
	SpawnedInstruments.Empty();
}

void ATutorialManager::SpawnDummyCharacterWithInstrument(EWeaponType WeaponType)
{
	if (!DummyCharacterClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("DummyCharacterClass is not set"));
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator Rotation = FRotator(0, 180, 0);
	ATutorialDummy* SpawnedDummy = GetWorld()->SpawnActor<ATutorialDummy>(DummyCharacterClass, DummyCharacterSpawnLocation, Rotation, SpawnParams);
	if (!SpawnedDummy)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to spawn dummy character"));
		return;
	}
	
	AInstrumentBase* SpawnedInstrument = SpawnInstrument(WeaponType);
	if (IsValid(SpawnedInstrument))
	{
		SpawnedDummy->Equip(SpawnedInstrument);
	}
}

void ATutorialManager::ActivateGimmicks()
{
	if (AGimmickManager* Manager = GetCachedGimmickManager())
	{
		Manager->ActivateAllGimmicks();
	}
}

void ATutorialManager::DeactivateGimmicks()
{
	if (AGimmickManager* Manager = GetCachedGimmickManager())
	{
		Manager->DeactivateAllGimmicks();
	}
}

void ATutorialManager::ShowTutorialCompletePopup() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		FTwoButtonPopupParams Params;
		Params.Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndTitle"));
		Params.Content = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndDescription"));
		Params.LeftButtonText = GI->GetCommonUIText(TEXT("Common_Yes"));
		Params.RightButtonText = GI->GetCommonUIText(TEXT("StringKey_Common_GoToMainMenu"));
		
		Params.LeftCallback = [this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
		};
		
		Params.RightCallback = [this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::MainMenu);
		};
		
		if (UTwoButtonPopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonPopup>(GetWorld()))
		{
			Popup->Init(Params);
		}
	}
}

ADefaultTromboneCharacter* ATutorialManager::GetCachedPlayerCharacter()
{
	if (CachedPlayerCharacter.IsValid())
	{
		return CachedPlayerCharacter.Get();
	}

	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		CachedPlayerCharacter = Cast<ADefaultTromboneCharacter>(Character);
		return CachedPlayerCharacter.Get();
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get player character"));
	return nullptr;
}

AGimmickManager* ATutorialManager::GetCachedGimmickManager()
{
	if (CachedGimmickManager.IsValid())
	{
		return CachedGimmickManager.Get();
	}

	if (AGimmickManager* Manager = Cast<AGimmickManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGimmickManager::StaticClass())))
	{
		CachedGimmickManager = Manager;
		return CachedGimmickManager.Get();
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get GimmickManager"));
	return nullptr;
}
